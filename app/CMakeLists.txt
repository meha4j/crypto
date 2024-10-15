add_executable(burro_exe burro.c)

target_link_libraries(burro_exe burro)
target_include_directories(burro_exe PUBLIC ${BURRO_INCLUDE_DIR})
target_compile_options(burro_exe PUBLIC ${COMPILE_OPTIONS})

set_property(TARGET burro_exe PROPERTY OUTPUT_NAME burro)

install(TARGETS burro_exe DESTINATION bin)
