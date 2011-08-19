file(GLOB src *.cpp *.hpp)
file(GLOB detail_src detail/*.hpp detail/*.cpp)
set(detail_fann_src
	detail/fann/doublefann.h
	detail/fann/doublefann.c
	detail/fann/fann_cpp.h)
	
add_definitions(-DFANN_NO_DLL -D_CRT_SECURE_NO_WARNINGS)
add_library(tcl.rll STATIC 
	${src}
	${detail_src}
	${detail_fann_src}
	detail/fann/doublefann.h
	detail/fann/doublefann.c)

source_group(rll FILES ${src})
source_group(rll\\detail FILES ${detail_src})
source_group(rll\\detail\\fann FILES ${detail_fann_src})

add_subdirectory(examples)
