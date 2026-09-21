#include "obsim/replay.hpp"
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <vector>

namespace obsim {

namespace {

std::vector<std::string> split(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream ss(line);
    std::string field;
    while (std::getline(ss, field, ',')) {
        fields.push_back(field);
    }
    return fields;
}

}  // namespace

std::string formatPrice(Price ticks) {
    std::ostringstream out;
    out << (ticks / 100) << '.' << std::setw(2) << std::setfill('0') << (ticks % 100);
    return out.str();
}

ReplayStats replayCsv(std::istream& in, OrderBook& book, std::ostream& tradeOut) {
    ReplayStats stats;
    std::uint64_t seq = 0;
    std::string line;

    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        ++stats.events;

        try {
            const auto f = split(line);
            if (f.size() == 6 && f[0] == "N") {
                Order order;
                order.id    = std::stoull(f[1]);
                order.side  = (f[2] == "B") ? Side::Buy : Side::Sell;
                order.type  = (f[3] == "M") ? OrderType::Market : OrderType::Limit;
                order.price = std::stoll(f[4]);
                order.qty   = static_cast<Quantity>(std::stoul(f[5]));
                order.seq   = ++seq;

                if ((f[2] != "B" && f[2] != "S") || (f[3] != "L" && f[3] != "M")) {
                    throw std::invalid_argument("bad side or type");
                }

                ++stats.newOrders;
                for (const Trade& t : book.addOrder(order)) {
                    ++stats.trades;
                    stats.volume += t.qty;
                    tradeOut << "TRADE buy=" << t.buyId << " sell=" << t.sellId
                             << " price=" << formatPrice(t.price)
                             << " qty=" << t.qty << '\n';
                }
            } else if (f.size() == 2 && f[0] == "C") {
                ++stats.cancels;
                if (!book.cancelOrder(std::stoull(f[1]))) ++stats.cancelMisses;
            } else {
                ++stats.badLines;
            }
        } catch (const std::exception&) {
            ++stats.badLines;   // non-numeric field, bad side/type, etc.
        }
    }
    return stats;
}

}  // namespace obsim