file(GLOB unittests *test.cpp)
add_executable(tcl.tests.unit_test ${unittests})
target_link_libraries(tcl.tests.unit_test ${Boost_LIBRARIES})
