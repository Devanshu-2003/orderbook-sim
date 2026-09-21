#pragma once
#include "obsim/types.hpp"

namespace obsim {

struct Order {
    OrderId   id;
    Side      side;
    OrderType type;
    Price     price;   // ignored for market orders
    Quantity  qty;     // remaining quantity
    std::uint64_t seq; // arrival order = time priority
};

}  // namespace obsim