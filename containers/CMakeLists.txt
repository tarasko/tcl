file(GLOB src *.cpp *.hpp)
add_library(tcl.containers STATIC ${src})

add_subdirectory(tests)
