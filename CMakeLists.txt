cmake_minimum_required(VERSION 3.16)
project(orderbook_sim CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(obsim_core src/orderbook.cpp src/replay.cpp src/generator.cpp)
target_include_directories(obsim_core PUBLIC include)
target_compile_options(obsim_core PRIVATE -Wall -Wextra)

add_executable(obsim src/main.cpp)
target_link_libraries(obsim obsim_core)

enable_testing()
add_subdirectory(tests)