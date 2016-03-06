cmake_minimum_required(VERSION 2.8.4)

################################################################################
####                           TEST                                         ####
################################################################################
enable_testing()
#set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)

project(ncd_unit_test)
include_directories(include ${GTEST_INCLUDE_DIRS} ../../include ../)
set(UNIT_TEST_FILES ncd_unit_test.cpp ncd_unit_test.hpp ../ncd.c)
find_package ( Threads REQUIRED GTest REQUIRED math REQUIRED)
add_executable(ncd_unit_test  ${UNIT_TEST_FILES})
target_link_libraries ( ncd_unit_test ${CMAKE_THREAD_LIBS_INIT} m gtest gtest_main)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
