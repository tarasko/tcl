file(GLOB src *.cpp *.hpp)
file(GLOB detail_src detail/*.hpp detail/*.cpp)
set(detail_fann_src_double
	detail/fann/doublefann.h
	detail/fann/doublefann.c
	detail/fann/fann_cpp.h)

set(detail_fann_src_int
	detail/fann/fixedfann.h
	detail/fann/fixedfann.c
	detail/fann/fann_cpp.h)
	
add_definitions(-DFANN_NO_DLL -D_CRT_SECURE_NO_WARNINGS)

add_library(tcl.rll.int STATIC 
	${src}
	${detail_src}
	${detail_fann_src_int})

add_library(tcl.rll.double STATIC 
	${src}
	${detail_src}
	${detail_fann_src_double})

set_target_properties(tcl.rll.int PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_INT")
set_target_properties(tcl.rll.double PROPERTIES COMPILE_DEFINITIONS "TCL_RLL_DOUBLE")

source_group(rll FILES ${src})
source_group(rll\\detail FILES ${detail_src})
source_group(rll\\detail\\fann_int FILES ${detail_fann_src_int})
source_group(rll\\detail\\fann_double FILES ${detail_fann_src_double})

add_subdirectory(examples)
