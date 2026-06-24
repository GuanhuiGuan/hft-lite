cmake_minimum_required(VERSION 3.12)
project(hft-lite)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_FLAGS "-std=c++26 -Wall -Wextra -Werror -Wpedantic --param destructive-interference-size=64")

find_package(Threads REQUIRED)

add_library(libcommon STATIC 
    src/socket_util.cpp
)
target_include_directories(libcommon PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_include_directories(libcommon PUBLIC ${PROJECT_SOURCE_DIR}/include/infra)

add_executable(socket socket.cpp)
target_link_libraries(socket PRIVATE 
    libcommon
    Threads::Threads
)