#include <fstream>
#include <iostream>
#include "obsim/orderbook.hpp"
#include "obsim/replay.hpp"
#include <exception>
#include <string>
#include "obsim/generator.hpp"

int main(int argc, char** argv) {
    if (argc >= 3 && std::string(argv[1]) == "--generate") {
        try {
            obsim::GeneratorConfig cfg;
            cfg.count = std::stoull(argv[2]);
            if (argc >= 4) cfg.seed = static_cast<unsigned>(std::stoul(argv[3]));
            obsim::generateEvents(cfg, std::cout);
            return 0;
        } catch (const std::exception&) {
            std::cerr << "usage: " << argv[0] << " --generate <count> [seed]\n";
            return 1;
        }
    }
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <events.csv>\n"
                  << "       " << argv[0] << " --generate <count> [seed] > events.csv\n";
        return 1;
    }
    std::ifstream file(argv[1]);
    if (!file) {
        std::cerr << "cannot open " << argv[1] << '\n';
        return 1;
    }

    obsim::OrderBook book;
    const obsim::ReplayStats s = obsim::replayCsv(file, book, std::cout);

    std::cout << "\n--- summary ---\n"
              << "events: " << s.events << "  (new " << s.newOrders
              << ", cancels " << s.cancels << ", cancel misses " << s.cancelMisses
              << ", bad lines " << s.badLines << ")\n"
              << "trades: " << s.trades << "  volume: " << s.volume << '\n'
              << "resting orders: " << book.restingOrderCount() << '\n'
              << "best bid: "
              << (book.bestBid() ? obsim::formatPrice(*book.bestBid()) : "none")
              << "  best ask: "
              << (book.bestAsk() ? obsim::formatPrice(*book.bestAsk()) : "none") << '\n';
    return 0;
}