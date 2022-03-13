cmake_minimum_required(VERSION 3.17)
project(archiver)

set(CMAKE_CXX_STANDARD 17)

include_directories(src/)

find_package(doctest REQUIRED)
add_executable(archiver_test test/test.cpp src/Huffman.cpp src/BitStream.cpp)
target_link_libraries(archiver_test PRIVATE doctest::doctest)

add_executable(archiver src/main.cpp src/Huffman.hpp src/Huffman.cpp src/BitStream.cpp)