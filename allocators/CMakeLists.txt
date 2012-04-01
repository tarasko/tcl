file(GLOB src *.cpp *.hpp)

add_subdirectory(alexandrescu)
add_subdirectory(tests)

add_custom_target(tcl.allocators SOURCES ${src})

