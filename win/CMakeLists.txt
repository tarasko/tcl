file(GLOB src *.cpp *.hpp)
add_library(tcl.win_s STATIC ${src})
add_library(tcl.win SHARED ${src})

add_subdirectory(tests)
