file(GLOB src *.cpp *.hpp)
add_library(tcl.lock_free STATIC ${src})

add_subdirectory(tests)
