file(GLOB src *.hpp)

add_subdirectory(allocators)
add_subdirectory(containers)
add_subdirectory(tests)

add_custom_target(tcl SOURCES ${src})

