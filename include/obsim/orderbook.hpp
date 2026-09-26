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
#include <cstdint>

namespace obsim {

enum class RejectReason {
    None,
    DuplicateId,     // an order with this ID is already resting
    ZeroQuantity,
    InvalidPrice     // limit order with price <= 0
};
// All resting orders at one price, oldest first (FIFO = time priority).
struct PriceLevel {
    std::list<Order> orders;
    Quantity totalQty = 0;
};

class OrderBook {
public:
    // Submit an order; returns any trades it caused.
    // Validates, matches, and appends any trades to `trades`.
    // Returns why the order was rejected, or None if it was accepted.
    RejectReason addOrder(Order order, std::vector<Trade>& trades);

    // Convenience form: returns the trades. A rejected order yields none.
    std::vector<Trade> addOrder(Order order);

    // The checks addOrder applies, exposed so callers and tests can use them.
    RejectReason validateOrder(const Order& order) const;
    // Cancel a resting order. Returns false if the ID isn't in the book.
    bool cancelOrder(OrderId id);

    std::optional<Price> bestBid() const;
    std::optional<Price> bestAsk() const;
    std::size_t restingOrderCount() const { return index_.size(); }
    void reserveOrders(std::size_t n) { index_.reserve(n); }
    std::uint64_t restingQuantity() const;   // total qty across all resting orders
    bool validate() const;              // internal consistency check (used by tests)

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