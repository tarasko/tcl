cmake_minimum_required(VERSION 2.8)

set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
add_definitions(-DBOOST_TEST_DYN_LINK)

find_package(Boost 1.49.0 COMPONENTS thread chrono system unit_test_framework)

include_directories(${Boost_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
link_directories(${Boost_LIBRARY_DIRS})

if (CMAKE_COMPILER_IS_GNUCXX )
    set(CMAKE_CXX_FLAGS "-std=c++0x")
endif(CMAKE_COMPILER_IS_GNUCXX)

file(GLOB src *.hpp)

add_subdirectory(allocators)
add_subdirectory(containers)
add_subdirectory(tests)

if(WIN32)
    add_subdirectory(win)
endif(WIN32)

add_custom_target(tcl SOURCES ${src})

