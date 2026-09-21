#pragma once
#include "obsim/types.hpp"

namespace obsim {

struct Trade {
    OrderId  buyId;
    OrderId  sellId;
    Price    price;
    Quantity qty;
};

}  // namespace obsim