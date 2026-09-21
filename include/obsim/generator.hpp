#pragma once
#include <cstdint>
#include <iosfwd>
#include "obsim/types.hpp"

namespace obsim {

struct GeneratorConfig {
    std::uint64_t count = 100000;   // number of events to emit
    unsigned seed = 1;
    Price startPrice = 10000;       // 100.00 in ticks
};

// Writes `count` events in the replay CSV format.
void generateEvents(const GeneratorConfig& cfg, std::ostream& out);

}  // namespace obsim