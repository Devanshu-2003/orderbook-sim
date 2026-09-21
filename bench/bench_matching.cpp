#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "obsim/generator.hpp"
#include "obsim/orderbook.hpp"

using namespace obsim;
using Clock = std::chrono::steady_clock;

namespace {

struct Event {
    bool  isCancel;
    Order order;   // used when !isCancel
    OrderId id;    // cancel target when isCancel
};

std::vector<Event> loadEvents(std::uint64_t count, unsigned seed) {
    GeneratorConfig cfg;
    cfg.count = count;
    cfg.seed = seed;
    std::ostringstream gen;
    generateEvents(cfg, gen);

    std::istringstream in(gen.str());
    std::vector<Event> events;
    events.reserve(count);
    std::uint64_t seq = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::vector<std::string> f;
        std::stringstream ss(line);
        std::string field;
        while (std::getline(ss, field, ',')) f.push_back(field);

        Event e{};
        if (f[0] == "C") {
            e.isCancel = true;
            e.id = std::stoull(f[1]);
        } else {
            e.isCancel = false;
            e.order.id    = std::stoull(f[1]);
            e.order.side  = (f[2] == "B") ? Side::Buy : Side::Sell;
            e.order.type  = (f[3] == "M") ? OrderType::Market : OrderType::Limit;
            e.order.price = std::stoll(f[4]);
            e.order.qty   = static_cast<Quantity>(std::stoul(f[5]));
            e.order.seq   = ++seq;
        }
        events.push_back(e);
    }
    return events;
}

// Untimed-per-op pass: whole-loop timing gives clean throughput.
double runThroughput(const std::vector<Event>& events, std::uint64_t& trades) {
    OrderBook book;
    trades = 0;
    const auto t0 = Clock::now();
    for (const Event& e : events) {
        if (e.isCancel) book.cancelOrder(e.id);
        else            trades += book.addOrder(e.order).size();
    }
    const auto t1 = Clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}

void report(const char* name, std::vector<std::uint64_t>& v) {
    if (v.empty()) return;
    std::sort(v.begin(), v.end());
    auto at = [&](double p) { return v[static_cast<std::size_t>(p * (v.size() - 1))]; };
    std::cout << std::left << std::setw(8) << name << " n=" << std::setw(9) << v.size()
              << " p50=" << at(0.50) << " ns   p99=" << at(0.99)
              << " ns   p99.9=" << at(0.999) << " ns   max=" << v.back() << " ns\n";
}

}  // namespace

int main(int argc, char** argv) {
    const std::uint64_t count = (argc >= 2) ? std::stoull(argv[1]) : 1000000;
    const unsigned seed = (argc >= 3) ? static_cast<unsigned>(std::stoul(argv[2])) : 1;

#ifndef NDEBUG
    std::cout << "WARNING: Debug build - these numbers are meaningless. Build with Release.\n";
#endif

    std::cout << "Generating and parsing " << count << " events (seed " << seed << ")...\n";
    const std::vector<Event> events = loadEvents(count, seed);

    std::uint64_t trades = 0;
    runThroughput(events, trades);                       // warm-up, discarded

    double best = 1e30;
    for (int run = 0; run < 3; ++run) {
        best = std::min(best, runThroughput(events, trades));
    }
    std::cout << std::fixed << std::setprecision(2)
              << "\nThroughput (best of 3): " << (events.size() / best / 1e6)
              << " M events/sec  (" << best << " s for " << events.size()
              << " events, " << trades << " trades)\n\n";

    // Per-operation latency pass.
    OrderBook book;
    std::vector<std::uint64_t> addNs, cancelNs;
    addNs.reserve(events.size());
    cancelNs.reserve(events.size());
    for (const Event& e : events) {
        const auto t0 = Clock::now();
        if (e.isCancel) book.cancelOrder(e.id);
        else            book.addOrder(e.order);
        const auto t1 = Clock::now();
        const auto ns = static_cast<std::uint64_t>(
            std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count());
        (e.isCancel ? cancelNs : addNs).push_back(ns);
    }
    std::cout << "Latency per operation (includes timer overhead):\n";
    report("add", addNs);
    report("cancel", cancelNs);
    return 0;
}