cmake_minimum_required(VERSION 3.16)
project(orderbook_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(OBSIM_SANITIZE "Build with AddressSanitizer + UBSan" OFF)
if(OBSIM_SANITIZE)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer)
    add_link_options(-fsanitize=address,undefined)
endif()

add_library(obsim_core src/orderbook.cpp src/replay.cpp src/generator.cpp)
target_include_directories(obsim_core PUBLIC include)
target_compile_options(obsim_core PRIVATE -Wall -Wextra)

add_executable(obsim src/main.cpp)
target_link_libraries(obsim obsim_core)

add_executable(bench_matching bench/bench_matching.cpp)
target_link_libraries(bench_matching obsim_core)

enable_testing()
add_subdirectory(tests)