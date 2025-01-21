cmake_minimum_required(VERSION 3.15)
project(stoat)

set(CMAKE_CXX_STANDARD 20)

add_executable(stoat src/main.cpp src/types.h src/core.h src/bitboard.h src/util/bits.h src/position.h src/position.cpp
	src/util/result.h src/util/split.h src/util/split.cpp src/util/parse.h)
