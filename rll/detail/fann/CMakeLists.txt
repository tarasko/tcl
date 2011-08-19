include_directories(fann)
file(GLOB hdrs fann/fann*.h)
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_library(fann STATIC 
	doublefann.c
	${hdrs}
	fann/doublefann.h)
