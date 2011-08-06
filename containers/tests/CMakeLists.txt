add_executable(tcl.containers.tests.performance performance.cpp)
target_link_libraries(tcl.containers.tests.performance ${Boost_LIBRARIES} tcl.containers)
