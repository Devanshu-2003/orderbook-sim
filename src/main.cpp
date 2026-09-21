#include <fstream>
#include <iostream>
#include "obsim/orderbook.hpp"
#include "obsim/replay.hpp"

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "usage: " << argv[0] << " <events.csv>\n";
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