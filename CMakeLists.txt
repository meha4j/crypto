cmake_minimum_required(VERSION 3.10)
project(burro)

add_executable(burro main.c)

target_link_libraries(burro unistring)

