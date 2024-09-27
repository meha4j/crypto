cmake_minimum_required(VERSION 3.10)
project(burro)

set(SRC
  src/key.h
  src/key.c
  
  src/burro.c
)

add_executable(burro ${SRC})

target_link_libraries(burro unistring)

