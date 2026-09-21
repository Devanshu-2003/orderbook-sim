include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(test_matching testmatch.cpp)
target_link_libraries(test_matching obsim_core GTest::gtest_main)
gtest_discover_tests(test_matching)

add_executable(test_invariants testinvariants.cpp)
target_link_libraries(test_invariants obsim_core GTest::gtest_main)
gtest_discover_tests(test_invariants)