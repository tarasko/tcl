file(GLOB src *.cpp *.hpp)
add_library(tcl.allocators STATIC ${src})

add_subdirectory(alexandrescu)
add_subdirectory(tests)
