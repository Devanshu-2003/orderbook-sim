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

TEST(Matching, NonCrossingOrdersRest) {
    OrderBook book;
    auto t1 = book.addOrder({1, Side::Sell, OrderType::Limit, 5300, 100, 1});
    auto t2 = book.addOrder({2, Side::Buy,  OrderType::Limit, 5200, 100, 2});
    EXPECT_TRUE(t1.empty());
    EXPECT_TRUE(t2.empty());
    ASSERT_TRUE(book.bestBid().has_value());
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestBid(), 5200);
    EXPECT_EQ(*book.bestAsk(), 5300);
}

TEST(Matching, PartialFillLeavesRemainderResting) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    auto trades = book.addOrder({2, Side::Buy, OrderType::Limit, 5200, 40, 2});
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].qty, 40u);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), 5200);            // 60 shares still resting
    EXPECT_FALSE(book.bestBid().has_value());
}

TEST(Matching, MarketOrderSweepsMultipleLevels) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5205, 150, 2});
    auto trades = book.addOrder({3, Side::Buy, OrderType::Market, 0, 200, 3});
    ASSERT_EQ(trades.size(), 2u);
    EXPECT_EQ(trades[0].price, 5200);
    EXPECT_EQ(trades[0].qty, 100u);
    EXPECT_EQ(trades[1].price, 5205);
    EXPECT_EQ(trades[1].qty, 100u);
    ASSERT_TRUE(book.bestAsk().has_value());
    EXPECT_EQ(*book.bestAsk(), 5205);            // 50 left at 5205
}

TEST(Matching, SamePriceFillsInArrivalOrder) {
    OrderBook book;
    book.addOrder({1, Side::Sell, OrderType::Limit, 5200, 100, 1});
    book.addOrder({2, Side::Sell, OrderType::Limit, 5200, 100, 2});
    auto trades = book.addOrder({3, Side::Buy, OrderType::Limit, 5200, 100, 3});
    ASSERT_EQ(trades.size(), 1u);
    EXPECT_EQ(trades[0].sellId, 1u);             // the earlier seller wins
}