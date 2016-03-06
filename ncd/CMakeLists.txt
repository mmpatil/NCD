cmake_minimum_required(VERSION 2.8.4)

project(ncd)
#set (ncd_VERSION_MAJOR 0)
#set (ncd_VERSION_MINOR 1)

include_directories(../include)
set(NCD_SOURCE_FILES ncd.c ncd_main.c ncd.c)
find_package ( Threads REQUIRED )
add_executable(ncd_main ${NCD_SOURCE_FILES})
target_link_libraries ( ncd_main ${CMAKE_THREAD_LIBS_INIT} m)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

add_subdirectory(Test)
################################################################################
####                           TEST                                         ####
################################################################################
#enable_testing()
#
#project(ncd_unit_test)
#include_directories(include Test ${GTEST_INCLUDE_DIRS})
#set(UNIT_TEST_FILES Test/ncd_unit_test.cpp Test/ncd_unit_test.hpp ncd.c)
#find_package ( Threads REQUIRED GTest REQUIRED math REQUIRED)
#add_executable(ncd_unit_test  ${UNIT_TEST_FILES})
#target_link_libraries ( ncd_unit_test ${CMAKE_THREAD_LIBS_INIT} m gtest gtest_main)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
