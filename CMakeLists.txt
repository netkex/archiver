cmake_minimum_required(VERSION 3.17)
project(hw_02)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/)

find_package(doctest REQUIRED)
add_executable(test htest/test.cpp src/Huffman.cpp src/BitStream.cpp)
target_link_libraries(test PRIVATE doctest::doctest)

add_executable(hw_02 src/main.cpp src/Huffman.hpp src/Huffman.cpp src/BitStream.cpp)