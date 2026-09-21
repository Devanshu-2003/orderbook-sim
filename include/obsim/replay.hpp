#pragma once
#include <cstdint>
#include <iosfwd>
#include <string>
#include "obsim/orderbook.hpp"

namespace obsim {

struct ReplayStats {
    std::uint64_t events = 0;
    std::uint64_t newOrders = 0;
    std::uint64_t cancels = 0;
    std::uint64_t cancelMisses = 0;
    std::uint64_t rejectedDuplicate = 0;
    std::uint64_t rejectedZeroQty = 0;
    std::uint64_t rejectedBadPrice = 0;   // cancel of an unknown or already-gone ID
    std::uint64_t trades = 0;
    std::uint64_t volume = 0;         // total quantity traded
    std::uint64_t badLines = 0;       // malformed input lines (skipped)
};

// 5200 -> "52.00"
std::string formatPrice(Price ticks);

// Reads events from `in`, applies them to `book`, writes one line per trade to `tradeOut`.
ReplayStats replayCsv(std::istream& in, OrderBook& book, std::ostream& tradeOut);

}  // namespace obsim