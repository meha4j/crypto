cmake_minimum_required(VERSION 3.10)
project(burro C)

set(CMAKE_EXPORT_COMPILE_COMMANDS true)
set(CMAKE_BUILD_TYPE Debug)
set(C_STANDARD 23)

set(SRC
  src/burro.c
  src/cphr.c
  src/uc.c
  src/ga.c
)

set(HDRS
  include/cphr.h
  include/ga.h
  include/uc.h
)

add_executable(burro ${SRC} ${HDRS})

target_include_directories(burro PUBLIC include/)

target_link_libraries(burro unistring)
target_compile_options(burro PUBLIC -std=c23)
# target_compile_options(burro PUBLIC -Wall -Wextra -Wpedantic -Werror)

