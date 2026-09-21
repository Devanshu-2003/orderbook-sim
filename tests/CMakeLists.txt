include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(test_matching testmatch.cpp)
target_link_libraries(test_matching obsim_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(test_matching)