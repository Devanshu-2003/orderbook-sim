#include "obsim/orderbook.hpp"
#include <algorithm>
#include <iterator>

namespace obsim {

std::uint64_t OrderBook::restingQuantity() const {
    std::uint64_t total = 0;
    for (const auto& entry : bids_) total += entry.second.totalQty;
    for (const auto& entry : asks_) total += entry.second.totalQty;
    return total;
}

bool OrderBook::validate() const {
    std::size_t count = 0;

    auto checkSide = [&](const auto& side, Side expectedSide) {
        for (const auto& entry : side) {
            const PriceLevel& level = entry.second;
            if (level.orders.empty()) return false;            // empty levels must be erased
            Quantity sum = 0;
            for (const Order& o : level.orders) {
                if (o.qty == 0 || o.price != entry.first || o.side != expectedSide) return false;
                const auto found = index_.find(o.id);
                if (found == index_.end() || &*found->second.it != &o) return false;
                sum += o.qty;
                ++count;
            }
            if (sum != level.totalQty) return false;
        }
        return true;
    };

    return checkSide(bids_, Side::Buy) && checkSide(asks_, Side::Sell) &&
           count == index_.size();
}


namespace {

// Remove one resting order from its price level; drop the level if it empties.
template <typename BookSide>
void removeResting(BookSide& side, Price price, std::list<Order>::iterator it) {
    auto levelIt = side.find(price);
    PriceLevel& level = levelIt->second;
    level.totalQty -= it->qty;
    level.orders.erase(it);
    if (level.orders.empty()) {
        side.erase(levelIt);
    }
}

}  // namespace

RejectReason OrderBook::validateOrder(const Order& order) const {
    if (order.qty == 0) return RejectReason::ZeroQuantity;
    if (order.type == OrderType::Limit && order.price <= 0) return RejectReason::InvalidPrice;
    if (index_.find(order.id) != index_.end()) return RejectReason::DuplicateId;
    return RejectReason::None;
}

RejectReason OrderBook::addOrder(Order order, std::vector<Trade>& trades) {
    const RejectReason reason = validateOrder(order);
    if (reason != RejectReason::None) return reason;   // book untouched

    if (order.side == Side::Buy) {
        matchAgainst(order, asks_, trades);
    } else {
        matchAgainst(order, bids_, trades);
    }

    // Anything left over from a limit order waits in the book.
    // Leftover market-order quantity is simply dropped.
    if (order.qty > 0 && order.type == OrderType::Limit) {
        rest(order);
    }
    return RejectReason::None;
}

std::vector<Trade> OrderBook::addOrder(Order order) {
    std::vector<Trade> trades;
    addOrder(order, trades);
    return trades;
}

bool OrderBook::cancelOrder(OrderId id) {
    auto found = index_.find(id);
    if (found == index_.end()) {
        return false;                 // unknown, already filled, or already cancelled
    }
    const Locator loc = found->second;
    if (loc.side == Side::Buy) {
        removeResting(bids_, loc.price, loc.it);
    } else {
        removeResting(asks_, loc.price, loc.it);
    }
    index_.erase(found);
    return true;
}

template <typename BookSide>
void OrderBook::matchAgainst(Order& incoming, BookSide& opposite,
                             std::vector<Trade>& trades) {
    while (incoming.qty > 0 && !opposite.empty()) {
        auto levelIt = opposite.begin();          // best price on the other side
        const Price levelPrice = levelIt->first;
        PriceLevel& level = levelIt->second;

        // A limit order only trades if the prices cross.
        if (incoming.type == OrderType::Limit) {
            const bool crosses = (incoming.side == Side::Buy)
                                     ? (levelPrice <= incoming.price)
                                     : (levelPrice >= incoming.price);
            if (!crosses) break;
        }

        // Walk the queue at this price, oldest order first.
        while (incoming.qty > 0 && !level.orders.empty()) {
            Order& resting = level.orders.front();
            const Quantity fill = std::min(incoming.qty, resting.qty);

            // The trade happens at the RESTING order's price.
            if (incoming.side == Side::Buy) {
                trades.push_back(Trade{incoming.id, resting.id, levelPrice, fill});
            } else {
                trades.push_back(Trade{resting.id, incoming.id, levelPrice, fill});
            }

            incoming.qty -= fill;
            resting.qty -= fill;
            level.totalQty -= fill;

            if (resting.qty == 0) {
                index_.erase(resting.id);
                level.orders.pop_front();
            }
        }

        if (level.orders.empty()) {
            opposite.erase(levelIt);              // remove the empty price level
        }
    }
}

void OrderBook::rest(const Order& order) {
    PriceLevel& level = (order.side == Side::Buy) ? bids_[order.price]
                                                  : asks_[order.price];
    level.orders.push_back(order);
    level.totalQty += order.qty;
    index_.emplace(order.id,
                   Locator{order.side, order.price, std::prev(level.orders.end())});
}

std::optional<Price> OrderBook::bestBid() const {
    if (bids_.empty()) return std::nullopt;
    return bids_.begin()->first;
}

std::optional<Price> OrderBook::bestAsk() const {
    if (asks_.empty()) return std::nullopt;
    return asks_.begin()->first;
}

}  // namespace obsim