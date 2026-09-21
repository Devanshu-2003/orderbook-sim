#include "obsim/generator.hpp"
#include <ostream>
#include <random>
#include <vector>
#include "obsim/orderbook.hpp"

namespace obsim {

void generateEvents(const GeneratorConfig& cfg, std::ostream& out) {
    std::mt19937_64 rng(cfg.seed);
    std::uniform_int_distribution<int> percent(0, 99);
    std::uniform_int_distribution<int> coin(0, 1);
    std::uniform_int_distribution<int> nudge(1, 2);
    std::uniform_int_distribution<Quantity> limitQty(1, 100);
    std::uniform_int_distribution<Quantity> marketQty(1, 50);
    std::geometric_distribution<int> depth(0.2);   // mean ~4 ticks from the anchor

    constexpr std::size_t kWindow = 2000;
    std::uniform_int_distribution<std::size_t> slot(0, kWindow - 1);

    OrderBook shadow;               // tells us which cancels are valid
    std::vector<OrderId> recent;    // recently placed limit orders = cancel candidates
    Price anchor = cfg.startPrice;
    OrderId nextId = 1;
    std::uint64_t seq = 0;

    out << "# type,id,side,ordertype,price,qty\n";

    for (std::uint64_t i = 0; i < cfg.count; ++i) {
        if (percent(rng) < 2) {
            anchor += coin(rng) ? nudge(rng) : -nudge(rng);
            if (anchor < 100) anchor = 100;
        }

        const int roll = percent(rng);

        // ~30%: try to cancel a recent order. If it already traded, fall through
        // and emit a new limit order instead, so every iteration emits one event.
        if (roll < 30 && !recent.empty()) {
            std::uniform_int_distribution<std::size_t> pick(0, recent.size() - 1);
            const std::size_t idx = pick(rng);
            const OrderId id = recent[idx];
            recent[idx] = recent.back();
            recent.pop_back();
            if (shadow.cancelOrder(id)) {
                out << "C," << id << '\n';
                continue;
            }
        }

        const bool isMarket = (roll >= 30 && roll < 40);

        Order o;
        o.id   = nextId++;
        o.side = coin(rng) ? Side::Buy : Side::Sell;
        o.seq  = ++seq;
        if (isMarket) {
            o.type  = OrderType::Market;
            o.price = 0;
            o.qty   = marketQty(rng);
        } else {
            o.type = OrderType::Limit;
            const Price d = depth(rng);
            o.price = (o.side == Side::Buy) ? anchor - d + 1 : anchor + d - 1;
            if (o.price < 1) o.price = 1;
            o.qty = limitQty(rng);
        }

        shadow.addOrder(o);

        if (!isMarket) {
            if (recent.size() < kWindow) recent.push_back(o.id);
            else                         recent[slot(rng)] = o.id;
        }

        out << "N," << o.id << ',' << (o.side == Side::Buy ? 'B' : 'S') << ','
            << (isMarket ? 'M' : 'L') << ',' << o.price << ',' << o.qty << '\n';
    }
}

}  // namespace obsim