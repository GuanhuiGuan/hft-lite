cmake_minimum_required(VERSION 3.10)
project(hft-lite)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++26 -Wall -Wextra -Werror -Wpedantic")
set(CMAKE_VERBOSE_MAKEFILE on)

add_executable(macros macros.cpp)
add_executable(thread thread.cpp)
add_executable(mempool mempool.cpp)
add_executable(lockfree lockfree.cpp)