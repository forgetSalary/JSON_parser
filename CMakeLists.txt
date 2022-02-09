cmake_minimum_required(VERSION 3.17)
project(JSON_parser C)

set(CMAKE_C_STANDARD 99)

#add_executable(JSON_parser main.c)
add_library(JSON_parser STATIC main.c)