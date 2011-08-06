add_executable(tcl.allocators.tests.performance performance.cpp)
target_link_libraries(tcl.allocators.tests.performance ${Boost_LIBRARIES} tcl.allocators.alexandrescu)

add_executable(tcl.allocators.tests.construct_destroy construct_destroy.cpp)
