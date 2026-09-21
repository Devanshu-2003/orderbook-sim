add_test([=[Invariants.RandomOrderFlowKeepsBookConsistent]=]  /Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests/test_invariants [==[--gtest_filter=Invariants.RandomOrderFlowKeepsBookConsistent]==] --gtest_also_run_disabled_tests)
set_tests_properties([=[Invariants.RandomOrderFlowKeepsBookConsistent]=]
  PROPERTIES
    
    DEF_SOURCE_LINE [==[/Users/devanshuchudhary/Desktop/orderbook-sim/tests/testinvariants.cpp:24]==]
    WORKING_DIRECTORY [==[/Users/devanshuchudhary/Desktop/orderbook-sim/build-asan/tests]==]
    SKIP_REGULAR_EXPRESSION [==[\[  SKIPPED \]]==]
    
)
set(test_invariants_TESTS [==[Invariants.RandomOrderFlowKeepsBookConsistent]==])
