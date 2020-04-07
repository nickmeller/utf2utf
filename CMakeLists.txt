cmake_minimum_required(VERSION 3.15)
project(utf2utf C)

set(CMAKE_C_STANDARD 11)

add_executable(utf2utf main.c encoding.h encoding.c)