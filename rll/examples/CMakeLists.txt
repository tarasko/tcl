macro(add_example name)
	file(GLOB src ${name}/*.hpp ${name}/*.cpp)
	add_executable(tcl.rll.examples.${name} ${src})
	target_link_libraries(tcl.rll.examples.${name} tcl.rll)
endmacro(add_example)

add_example(grid_world_action)
add_example(grid_world_state)
add_example(tic_tac_toe)
