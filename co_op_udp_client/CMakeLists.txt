cmake_minimum_required(VERSION 2.8.4)

project(co_op_udp_client)
#set (ncd_VERSION_MAJOR 0)
#set (ncd_VERSION_MINOR 1)
set (Boost_USE_STATIC_LIBS ON)
find_package(Threads REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(../include)
set(CLIENT_SOURCE_FILES client.cpp ../include/co_op_udp_detector.hpp)
SET(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
SET(BUILD_SHARED_LIBRARIES OFF)
SET(CMAKE_EXE_LINKER_FLAGS "-static")
add_executable(client ${CLIENT_SOURCE_FILES})
target_compile_features(client PUBLIC cxx_strong_enums cxx_thread_local)
set_target_properties(client PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT} m ${Boost_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

#add_subdirectory(Test)
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
