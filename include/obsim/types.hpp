#pragma once
#include <cstdint>

namespace obsim {

using OrderId  = std::uint64_t;
using Price    = std::int64_t;   // in ticks: 5205 means 52.05
using Quantity = std::uint32_t;

enum class Side { Buy, Sell };
enum class OrderType { Limit, Market };

}  // namespace obsim