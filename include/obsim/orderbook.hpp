#pragma once
#include <vector>
#include "obsim/orders.hpp"
#include "obsim/trade.hpp"

namespace obsim {

class OrderBook {
public:
    // Submit an order; returns any trades it caused.
    std::vector<Trade> addOrder(Order order);
};

}