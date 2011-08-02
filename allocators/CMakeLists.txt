file(GLOB src *.cpp *.hpp)

add_subdirectory(alexandrescu)
add_subdirectory(tests)

add_library(tcl.allocators STATIC EXCLUDE_FROM_ALL ${src})

