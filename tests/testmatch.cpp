#include <gtest/gtest.h>
#include "obsim/orderbook.hpp"
#include <sstream>
#include "obsim/replay.hpp"
#include "obsim/generator.hpp"
#include <vector>
using namespace obsim;

TEST(Matching, BuyCrossesRestingSell) {
    OrderBook book;

    // A seller waits: sell 100 @ 52.00 (5200 ticks)
    auto t1 = book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    EXPECT_TRUE(t1.empty());   // nothing to match against yet

    // A buyer arrives willing to pay 52.00: buy 100 @ 52.00
    auto t2 = book.addOrder({2, Side::Buy, OrderType::Limit, 5200, 100, 2});

    ASSERT_EQ(t2.size(), 1u);
    EXPECT_EQ(t2[0].price, 5200);
    EXPECT_EQ(t2[0].qty, 100u);
    EXPECT_EQ(t2[0].buyId, 2u);
    EXPECT_EQ(t2[0].sellId, 1u);
}

TEST(Matching, NonCrossingOrdersRest) {
    OrderBook book;
    auto t1 = book.addOrder({1, Side::Sell, OrderType::Limit, 5300, 100, 1});
    auto t2 = book.addOrder({2, Side::Buy,  OrderType::Limit, 5200, 100, 2});
    EXPECT_TRUE(t1.empty());
    EXPECT_TRUE(t2.empty());
    ASSERT_TRUE(book.bestBid().has_value());
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestBid(), 5200);
    EXPECT_EQ(*book.bestAsk(), 5300);
}

TEST(Matching, PartialFillLeavesRemainderResting) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    auto trades = book.addOrder({2, Side::Buy, OrderType::Limit, 5200, 40, 2});
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].qty, 40u);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), 5200);            // 60 shares still resting
    EXPECT_FALSE(book.bestBid().has_value());
}

TEST(Matching, MarketOrderSweepsMultipleLevels) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5205, 150, 2});
    auto trades = book.addOrder({3, Side::Buy, OrderType::Market, 0, 200, 3});
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].price, 5200);
    EXPECT_EQ(trades[0].qty, 100u);
    EXPECT_EQ(trades[1].price, 5205);
    EXPECT_EQ(trades[1].qty, 100u);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), 5205);            // 50 left at 5205
}

TEST(Matching, SamePriceFillsInArrivalOrder) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5200, 100, 2});
    auto trades = book.addOrder({3, Side::Buy, OrderType::Limit, 5200, 100, 3});
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].sellId, 1u);             // the earlier seller wins
}

TEST(Cancel, RemovesRestingOrder) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    EXPECT_TRUE(book.cancelOrder(1));
    EXPECT_FALSE(book.bestAsk().has_value());
    EXPECT_EQ(book.restingOrderCount(), 0u);
}

TEST(Cancel, UnknownIdReturnsFalse) {
    OrderBook book;
    EXPECT_FALSE(book.cancelOrder(999));
}

TEST(Cancel, SecondCancelOfSameIdReturnsFalse) {
    OrderBook book;
    book.addOrder({1, Side::Buy, OrderType::Limit, 5200, 100, 1});
    EXPECT_TRUE(book.cancelOrder(1));
    EXPECT_FALSE(book.cancelOrder(1));
}

TEST(Cancel, FullyFilledOrderCannotBeCancelled) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Buy,  OrderType::Limit, 5200, 100, 2});  // fills order 1
    EXPECT_FALSE(book.cancelOrder(1));
}

TEST(Cancel, MiddleOfQueueKeepsOthersInOrder) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5200, 100, 2});
    book.addOrder({3, Side::Sell, OrderType::Limit, 5200, 100, 3});
    EXPECT_TRUE(book.cancelOrder(2));

    auto trades = book.addOrder({4, Side::Buy, OrderType::Market, 0, 200, 4});
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].sellId, 1u);
    EXPECT_EQ(trades[1].sellId, 3u);             // order 2 is gone; 3 keeps its place
}

TEST(Cancel, CancelledOrderIsNotMatched) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.cancelOrder(1);
    auto trades = book.addOrder({2, Side::Buy, OrderType::Limit, 5200, 100, 2});
    EXPECT_TRUE(trades.empty());
    ASSERT_TRUE(book.bestBid().has_value());     // the buy rests instead
}

TEST(Cancel, OnlyEmptiesItsOwnPriceLevel) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5205, 100, 2});
    book.cancelOrder(1);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), 5205);            // next level becomes the best ask
}


TEST(Replay, RunsEventsAndCountsTrades) {
    std::istringstream input(
        "# comment\n"
        "N,1,S,L,5200,100\n"
        "N,2,B,L,5200,100\n"
        "C,42\n"
        "garbage line\n");
    std::ostringstream out;
    OrderBook book;
    ReplayStats s = replayCsv(input, book, out);

    EXPECT_EQ(s.newOrders, 2u);
    EXPECT_EQ(s.trades, 1u);
    EXPECT_EQ(s.volume, 100u);
    EXPECT_EQ(s.cancelMisses, 1u);
    EXPECT_EQ(s.badLines, 1u);
    EXPECT_EQ(out.str(), "TRADE buy=2 sell=1 price=52.00 qty=100\n");
}

TEST(Generator, SameSeedProducesIdenticalOutput) {
    GeneratorConfig cfg;
    cfg.count = 2000;
    cfg.seed = 7;
    std::ostringstream a, b, c;
    generateEvents(cfg, a);
    generateEvents(cfg, b);
    EXPECT_EQ(a.str(), b.str());
    cfg.seed = 8;
    generateEvents(cfg, c);
    EXPECT_NE(a.str(), c.str());
}

TEST(Generator, OutputReplaysWithoutErrors) {
    GeneratorConfig cfg;
    cfg.count = 20000;
    cfg.seed = 3;
    std::ostringstream gen;
    generateEvents(cfg, gen);

    std::istringstream in(gen.str());
    std::ostringstream trades;
    OrderBook book;
    ReplayStats s = replayCsv(in, book, trades);

    EXPECT_EQ(s.events, 20000u);
    EXPECT_EQ(s.badLines, 0u);
    EXPECT_EQ(s.cancelMisses, 0u);   // shadow book guarantees valid cancels
    EXPECT_GT(s.trades, 100u);       // the flow really does cross and trade
    EXPECT_TRUE(book.validate());
}

TEST(Validation, DuplicateRestingIdIsRejectedAndBookStaysIntact) {
    OrderBook book;
    std::vector<Trade> trades;
    ASSERT_EQ(book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1}, trades),
              RejectReason::None);

    // Same ID again, other side, crossing price: must be rejected, not matched or rested.
    EXPECT_EQ(book.addOrder({1, Side::Buy, OrderType::Limit, 5300, 50, 2}, trades),
              RejectReason::DuplicateId);
    EXPECT_TRUE(trades.empty());
    EXPECT_EQ(book.restingOrderCount(), 1u);
    EXPECT_FALSE(book.bestBid().has_value());
    EXPECT_TRUE(book.validate());

    // The original order is still cancellable, and the book ends up empty.
    EXPECT_TRUE(book.cancelOrder(1));
    EXPECT_EQ(book.restingOrderCount(), 0u);
    EXPECT_FALSE(book.bestAsk().has_value());
}

TEST(Validation, ZeroQuantityIsRejected) {
    OrderBook book;
    std::vector<Trade> trades;
    EXPECT_EQ(book.addOrder({1, Side::Buy, OrderType::Limit, 5200, 0, 1}, trades),
              RejectReason::ZeroQuantity);
    EXPECT_EQ(book.addOrder({2, Side::Sell, OrderType::Market, 0, 0, 2}, trades),
              RejectReason::ZeroQuantity);
    EXPECT_EQ(book.restingOrderCount(), 0u);
}

TEST(Validation, NonPositiveLimitPriceRejectedButMarketPriceIgnored) {
    OrderBook book;
    std::vector<Trade> trades;
    EXPECT_EQ(book.addOrder({1, Side::Buy, OrderType::Limit, 0, 10, 1}, trades),
              RejectReason::InvalidPrice);
    EXPECT_EQ(book.addOrder({2, Side::Buy, OrderType::Limit, -5, 10, 2}, trades),
              RejectReason::InvalidPrice);
    // A market order's price field is unused, so 0 is fine.
    EXPECT_EQ(book.addOrder({3, Side::Buy, OrderType::Market, 0, 10, 3}, trades),
              RejectReason::None);
    EXPECT_EQ(book.restingOrderCount(), 0u);
}

TEST(Validation, IdCanBeReusedOnceOrderIsGone) {
    OrderBook book;
    std::vector<Trade> trades;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1}, trades);
    book.addOrder({2, Side::Buy,  OrderType::Limit, 5200, 100, 2}, trades);  // fills order 1
    ASSERT_EQ(book.restingOrderCount(), 0u);

    EXPECT_EQ(book.addOrder({1, Side::Sell, OrderType::Limit, 5300, 10, 3}, trades),
              RejectReason::None);
    EXPECT_EQ(book.restingOrderCount(), 1u);
}

TEST(Replay, CountsRejectsAndTreatsNegativeNumbersAsBadLines) {
    std::istringstream input(
        "N,1,S,L,5200,100\n"
        "N,1,B,L,5300,50\n"     // duplicate id
        "N,2,B,L,5200,0\n"      // zero quantity
        "N,3,B,L,0,10\n"        // limit price 0
        "N,4,B,L,5200,-5\n"     // negative quantity -> malformed line
        "N,-7,B,L,5200,5\n");   // negative id -> malformed line
    std::ostringstream out;
    OrderBook book;
    ReplayStats s = replayCsv(input, book, out);

    EXPECT_EQ(s.events, 6u);
    EXPECT_EQ(s.rejectedDuplicate, 1u);
    EXPECT_EQ(s.rejectedZeroQty, 1u);
    EXPECT_EQ(s.rejectedBadPrice, 1u);
    EXPECT_EQ(s.badLines, 2u);
    EXPECT_EQ(s.trades, 0u);
    EXPECT_EQ(book.restingOrderCount(), 1u);   // only the first order rests
}