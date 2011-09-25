file(GLOB src *.cpp *.hpp)
file(GLOB policy_src policy/*.hpp policy/*.cpp)
file(GLOB value_function_src value_function/*.hpp value_function/*.cpp)
file(GLOB detail_src detail/*.hpp detail/*.cpp)
set(detail_fann_src
	detail/fann/doublefann.h
	detail/fann/doublefann.c
	detail/fann/fann_cpp.h)

set(value_function_src_double ${value_function_src})
set(value_function_src_int ${value_function_src})

list(REMOVE_ITEM value_function_src_double
	"${CMAKE_CURRENT_SOURCE_DIR}/value_function/lookup_table.hpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/value_function/lookup_table.cpp")

add_definitions(-DFANN_NO_DLL -D_CRT_SECURE_NO_WARNINGS)

add_library(tcl.rll.int STATIC
	${src}
	${policy_src}
	${value_function_src_int}
	${detail_src}
	${detail_fann_src})

add_library(tcl.rll.double STATIC
	${src}
	${policy_src}
	${value_function_src_double}
	${detail_src}
	${detail_fann_src})

set_target_properties(tcl.rll.int PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_INT")
set_target_properties(tcl.rll.double PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_DOUBLE")

source_group(rll FILES ${src})
source_group(rll\\policy FILES ${policy_src})
source_group(rll\\value_function FILES ${value_function_src})
source_group(rll\\detail FILES ${detail_src})
source_group(rll\\detail\\fann FILES ${detail_fann_src})

add_subdirectory(examples)
