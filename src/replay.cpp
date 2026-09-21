#include "obsim/replay.hpp"
#include <iomanip>
#include <istream>
#include <ostream>
#include <sstream>
#include <vector>
#include <limits>

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
    std::vector<Trade> tradeBuf;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        ++stats.events;

        try {
            const auto f = split(line);
            if (f.size() == 6 && f[0] == "N") {
                if ((f[2] != "B" && f[2] != "S") || (f[3] != "L" && f[3] != "M")) {
                    throw std::invalid_argument("bad side or type");
                }
                const long long id  = std::stoll(f[1]);
                const long long qty = std::stoll(f[5]);
                if (id < 0 || qty < 0 ||
                    qty > static_cast<long long>(std::numeric_limits<Quantity>::max())) {
                    throw std::invalid_argument("id or quantity out of range");
                }

                Order order;
                order.id    = static_cast<OrderId>(id);
                order.side  = (f[2] == "B") ? Side::Buy : Side::Sell;
                order.type  = (f[3] == "M") ? OrderType::Market : OrderType::Limit;
                order.price = std::stoll(f[4]);
                order.qty   = static_cast<Quantity>(qty);
                order.seq   = ++seq;

                ++stats.newOrders;
                tradeBuf.clear();
                const RejectReason reason = book.addOrder(order, tradeBuf);
                if (reason == RejectReason::DuplicateId)       ++stats.rejectedDuplicate;
                else if (reason == RejectReason::ZeroQuantity) ++stats.rejectedZeroQty;
                else if (reason == RejectReason::InvalidPrice) ++stats.rejectedBadPrice;

                for (const Trade& t : tradeBuf) {
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