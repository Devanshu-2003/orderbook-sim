add_test([=[Matching.BuyCrossesRestingSell]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Matching.BuyCrossesRestingSell]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Matching.BuyCrossesRestingSell]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:9]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Matching.NonCrossingOrdersRest]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Matching.NonCrossingOrdersRest]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Matching.NonCrossingOrdersRest]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:26]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Matching.PartialFillLeavesRemainderResting]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Matching.PartialFillLeavesRemainderResting]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Matching.PartialFillLeavesRemainderResting]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:38]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Matching.MarketOrderSweepsMultipleLevels]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Matching.MarketOrderSweepsMultipleLevels]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Matching.MarketOrderSweepsMultipleLevels]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:49]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Matching.SamePriceFillsInArrivalOrder]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Matching.SamePriceFillsInArrivalOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Matching.SamePriceFillsInArrivalOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:63]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.RemovesRestingOrder]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.RemovesRestingOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.RemovesRestingOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:72]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.UnknownIdReturnsFalse]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.UnknownIdReturnsFalse]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.UnknownIdReturnsFalse]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:80]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.SecondCancelOfSameIdReturnsFalse]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.SecondCancelOfSameIdReturnsFalse]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.SecondCancelOfSameIdReturnsFalse]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:85]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.FullyFilledOrderCannotBeCancelled]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.FullyFilledOrderCannotBeCancelled]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.FullyFilledOrderCannotBeCancelled]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:92]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.MiddleOfQueueKeepsOthersInOrder]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.MiddleOfQueueKeepsOthersInOrder]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.MiddleOfQueueKeepsOthersInOrder]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:99]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.CancelledOrderIsNotMatched]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.CancelledOrderIsNotMatched]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.CancelledOrderIsNotMatched]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:112]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Cancel.OnlyEmptiesItsOwnPriceLevel]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Cancel.OnlyEmptiesItsOwnPriceLevel]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Cancel.OnlyEmptiesItsOwnPriceLevel]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:121]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Replay.RunsEventsAndCountsTrades]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Replay.RunsEventsAndCountsTrades]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Replay.RunsEventsAndCountsTrades]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:131]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Replay.CountsRejectsAndTreatsNegativeNumbersAsBadLines]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Replay.CountsRejectsAndTreatsNegativeNumbersAsBadLines]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Replay.CountsRejectsAndTreatsNegativeNumbersAsBadLines]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:237]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Generator.SameSeedProducesIdenticalOutput]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Generator.SameSeedProducesIdenticalOutput]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Generator.SameSeedProducesIdenticalOutput]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:150]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Generator.OutputReplaysWithoutErrors]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Generator.OutputReplaysWithoutErrors]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Generator.OutputReplaysWithoutErrors]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:163]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validation.DuplicateRestingIdIsRejectedAndBookStaysIntact]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Validation.DuplicateRestingIdIsRejectedAndBookStaysIntact]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validation.DuplicateRestingIdIsRejectedAndBookStaysIntact]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:182]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validation.ZeroQuantityIsRejected]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Validation.ZeroQuantityIsRejected]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validation.ZeroQuantityIsRejected]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:202]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validation.NonPositiveLimitPriceRejectedButMarketPriceIgnored]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Validation.NonPositiveLimitPriceRejectedButMarketPriceIgnored]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validation.NonPositiveLimitPriceRejectedButMarketPriceIgnored]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:212]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
add_test([=[Validation.IdCanBeReusedOnceOrderIsGone]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_matching [==[--gtest_filter=Validation.IdCanBeReusedOnceOrderIsGone]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Validation.IdCanBeReusedOnceOrderIsGone]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testmatch.cpp:225]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(test_matching_TESTS [==[Matching.BuyCrossesRestingSell]==] [==[Matching.NonCrossingOrdersRest]==] [==[Matching.PartialFillLeavesRemainderResting]==] [==[Matching.MarketOrderSweepsMultipleLevels]==] [==[Matching.SamePriceFillsInArrivalOrder]==] [==[Cancel.RemovesRestingOrder]==] [==[Cancel.UnknownIdReturnsFalse]==] [==[Cancel.SecondCancelOfSameIdReturnsFalse]==] [==[Cancel.FullyFilledOrderCannotBeCancelled]==] [==[Cancel.MiddleOfQueueKeepsOthersInOrder]==] [==[Cancel.CancelledOrderIsNotMatched]==] [==[Cancel.OnlyEmptiesItsOwnPriceLevel]==] [==[Replay.RunsEventsAndCountsTrades]==] [==[Replay.CountsRejectsAndTreatsNegativeNumbersAsBadLines]==] [==[Generator.SameSeedProducesIdenticalOutput]==] [==[Generator.OutputReplaysWithoutErrors]==] [==[Validation.DuplicateRestingIdIsRejectedAndBookStaysIntact]==] [==[Validation.ZeroQuantityIsRejected]==] [==[Validation.NonPositiveLimitPriceRejectedButMarketPriceIgnored]==] [==[Validation.IdCanBeReusedOnceOrderIsGone]==])
