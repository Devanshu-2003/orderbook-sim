#include "obsim/orderbook.hpp"
#include <algorithm>
#include <iterator>

namespace obsim {

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

std::vector<Trade> OrderBook::addOrder(Order order) {
    std::vector<Trade> trades;

    // A buy matches against sellers (asks); a sell matches against buyers (bids).
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