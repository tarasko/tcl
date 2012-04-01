file(GLOB unittests *test.cpp)
add_executable(tcl.win.tests.unit_test ${unittests})
target_link_libraries(tcl.win.tests.unit_test ${Boost_LIBRARIES} tcl.win_s)
