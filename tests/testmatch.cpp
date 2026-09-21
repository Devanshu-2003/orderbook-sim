#include <gtest/gtest.h>
#include "obsim/orderbook.hpp"

using namespace obsim;

TEST(Matching, BuyCrossesRestingSell) {
    OrderBook book;

    // A seller waits: sell 100 @ 52.00 (5200 ticks)
    auto t1 = book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    EXPECT_TRUE(t1.empty());   // nothing to match against yet

    // A buyer arrives willing to pay 52.00: buy 100 @ 52.00
    auto t2 = book.addOrder({2, Side::Buy, OrderType::Limit, 5200, 100, 2});

    ASSERT_EQ(t2.size(), 1u);
    EXPECT_EQ(t2[0].price, 5200);
    EXPECT_EQ(t2[0].qty, 100u);
    EXPECT_EQ(t2[0].buyId, 2u);
    EXPECT_EQ(t2[0].sellId, 1u);
}