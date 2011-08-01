file(GLOB src *.cpp *.hpp)
add_library(tcl.allocators.alexandrescu STATIC ${src})
