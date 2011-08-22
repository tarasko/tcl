file(GLOB src *.cpp *.hpp)
file(GLOB detail_src detail/*.hpp detail/*.cpp)
set(detail_fann_src
	detail/fann/doublefann.h
	detail/fann/doublefann.c
	detail/fann/fann_cpp.h)

set(src_double ${src})
set(src_int ${src})

list(REMOVE_ITEM src_double 
	"${CMAKE_CURRENT_SOURCE_DIR}/lookup_table.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/lookup_table.cpp")

add_definitions(-DFANN_NO_DLL -D_CRT_SECURE_NO_WARNINGS)

add_library(tcl.rll.int STATIC 
	${src_int}
	${detail_src}
	${detail_fann_src})

add_library(tcl.rll.double STATIC 
	${src_double}
	${detail_src}
	${detail_fann_src})

set_target_properties(tcl.rll.int PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_INT")
set_target_properties(tcl.rll.double PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_DOUBLE")

source_group(rll FILES ${src})
source_group(rll\\detail FILES ${detail_src})
source_group(rll\\detail\\fann FILES ${detail_fann_src})

add_subdirectory(examples)
