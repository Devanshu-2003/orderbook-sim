#include <gtest/gtest.h>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>
#include "obsim/orderbook.hpp"

using namespace obsim;

namespace {

// A deliberately simple model of the book, kept by the test itself.
struct Model {
    std::unordered_map<OrderId, Quantity> live;   // resting orders -> remaining qty
    std::uint64_t submitted = 0;
    std::uint64_t traded = 0;
    std::uint64_t cancelled = 0;
    std::uint64_t dropped = 0;                    // unfilled market-order remainder
};

}  // namespace

TEST(Invariants, RandomOrderFlowKeepsBookConsistent) {
    std::uint64_t grandTraded = 0;

    for (unsigned seed = 1; seed <= 25; ++seed) {
        std::mt19937 rng(seed);
        OrderBook book;
        Model m;
        OrderId nextId = 1;
        std::uint64_t seq = 0;

        std::uniform_int_distribution<int> percent(0, 99);
        std::uniform_int_distribution<int> coin(0, 1);
        std::uniform_int_distribution<Price> priceDist(5190, 5210);   // narrow band -> lots of crossing
        std::uniform_int_distribution<Quantity> qtyDist(1, 100);

        for (int step = 0; step < 4000; ++step) {
            SCOPED_TRACE("seed " + std::to_string(seed) + ", step " + std::to_string(step));
            const int roll = percent(rng);

            if (roll < 5 && !m.live.empty()) {
                // Resubmit an ID that is currently resting: must be rejected, changing nothing.
                const OrderId dup = m.live.begin()->first;
                std::vector<Trade> none;
                ASSERT_EQ(book.addOrder({dup, Side::Buy, OrderType::Limit,
                                         priceDist(rng), qtyDist(rng), ++seq}, none),
                          RejectReason::DuplicateId);
                ASSERT_TRUE(none.empty());
            } else if (roll < 30) {
                // Cancel a random ID (sometimes live, sometimes filled/never existed).
                std::uniform_int_distribution<OrderId> idDist(1, nextId);
                const OrderId id = idDist(rng);
                const auto it = m.live.find(id);
                const bool expected = (it != m.live.end());
                ASSERT_EQ(book.cancelOrder(id), expected);
                if (expected) {
                    m.cancelled += it->second;
                    m.live.erase(it);
                }
            } else {
                Order o;
                o.id    = nextId++;
                o.side  = coin(rng) ? Side::Buy : Side::Sell;
                o.type  = (roll < 40) ? OrderType::Market : OrderType::Limit;
                o.price = (o.type == OrderType::Limit) ? priceDist(rng) : 0;
                o.qty   = qtyDist(rng);
                o.seq   = ++seq;
                m.submitted += o.qty;

                const auto trades = book.addOrder(o);

                Quantity filled = 0;
                Price lastPrice = 0;
                bool first = true;
                for (const Trade& t : trades) {
                    filled += t.qty;
                    m.traded += t.qty;

                    // The incoming order must be on its own side of the trade.
                    ASSERT_EQ(o.side == Side::Buy ? t.buyId : t.sellId, o.id);

                    // The other party must be a known resting order with enough quantity.
                    const OrderId counter = (o.side == Side::Buy) ? t.sellId : t.buyId;
                    auto cit = m.live.find(counter);
                    ASSERT_NE(cit, m.live.end());
                    ASSERT_GE(cit->second, t.qty);
                    cit->second -= t.qty;
                    if (cit->second == 0) m.live.erase(cit);

                    // Prices respect the limit, and sweep from best to worse.
                    if (o.type == OrderType::Limit) {
                        if (o.side == Side::Buy) ASSERT_LE(t.price, o.price);
                        else                     ASSERT_GE(t.price, o.price);
                    }
                    if (!first) {
                        if (o.side == Side::Buy) ASSERT_GE(t.price, lastPrice);
                        else                     ASSERT_LE(t.price, lastPrice);
                    }
                    lastPrice = t.price;
                    first = false;
                }
                ASSERT_LE(filled, o.qty);

                if (o.type == OrderType::Limit) {
                    if (filled < o.qty) m.live[o.id] = o.qty - filled;   // remainder rests
                } else {
                    m.dropped += o.qty - filled;
                }
            }

            // ---- invariants, checked after every operation ----
            ASSERT_TRUE(book.validate());
            ASSERT_EQ(book.restingOrderCount(), m.live.size());

            std::uint64_t resting = 0;
            for (const auto& kv : m.live) resting += kv.second;
            ASSERT_EQ(book.restingQuantity(), resting);
            ASSERT_EQ(m.submitted, 2 * m.traded + resting + m.cancelled + m.dropped);

            if (book.bestBid() && book.bestAsk()) {
                ASSERT_LT(*book.bestBid(), *book.bestAsk());   // never crossed
            }
        }
        grandTraded += m.traded;
    }

    EXPECT_GT(grandTraded, 1000u);   // sanity: the test actually exercised matching
}