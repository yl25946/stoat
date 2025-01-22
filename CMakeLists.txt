cmake_minimum_required(VERSION 3.15)

file(STRINGS version.txt ST_VERSION LIMIT_COUNT 1)
project(stoat VERSION ${ST_VERSION})

set(CMAKE_CXX_STANDARD 20)

add_executable(stoat-native src/main.cpp src/types.h src/core.h src/bitboard.h src/util/bits.h src/position.h
	src/position.cpp src/util/result.h src/util/split.h src/util/split.cpp src/util/parse.h src/move.h src/move.cpp
	src/usi.h src/usi.cpp src/util/string_map.h src/attacks/attacks.h src/util/multi_array.h src/movegen.h
	src/util/static_vector.h src/movegen.cpp src/perft.h src/perft.cpp src/util/timer.h src/util/timer.cpp)

target_compile_options(stoat-native PUBLIC -march=native $<$<CONFIG:Release>:-flto>)
target_compile_definitions(stoat-native PUBLIC ST_VERSION=${CMAKE_PROJECT_VERSION})
