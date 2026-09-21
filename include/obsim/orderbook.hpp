#pragma once
#include <cstddef>
#include <functional>
#include <list>
#include <map>
#include <optional>
#include <unordered_map>
#include <vector>
#include "obsim/orders.hpp"
#include "obsim/trade.hpp"
#include "obsim/types.hpp"

namespace obsim {

// All resting orders at one price, oldest first (FIFO = time priority).
struct PriceLevel {
    std::list<Order> orders;
    Quantity totalQty = 0;
};

class OrderBook {
public:
    // Submit an order; returns any trades it caused.
    std::vector<Trade> addOrder(Order order);
    // Cancel a resting order. Returns false if the ID isn't in the book.
    bool cancelOrder(OrderId id);

    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;
    std::size_t restingOrderCount() const { return index_.size(); }

private:
    // Remembers where a resting order lives, so cancel can find it in O(1).
    struct Locator {
        Side side;
        Price price;
        std::list<Order>::iterator it;
    };

    template <typename BookSide>
    void matchAgainst(Order& incoming, BookSide& opposite, std::vector<Trade>& trades);
    void rest(const Order& order);

    std::map<Price, PriceLevel, std::greater<Price>> bids_;  // best bid = begin()
    std::map<Price, PriceLevel> asks_;                       // best ask = begin()
    std::unordered_map<OrderId, Locator> index_;
};

}  // namespace obsim