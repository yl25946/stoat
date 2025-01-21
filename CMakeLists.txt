cmake_minimum_required(VERSION 3.15)
project(stoat)

set(CMAKE_CXX_STANDARD 20)

add_executable(stoat src/main.cpp src/types.h
	src/core.h)
