cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH cmake)

project(liuri)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -pedantic -error")

include_directories(src)
add_subdirectory(src)
add_subdirectory(test)
