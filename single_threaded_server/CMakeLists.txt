cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

project(co_op_udp_server)

#set(Boost_USE_STATIC_LIBS ON)
find_package(Threads REQUIRED)
#find_package(Boost 1.6 COMPONENTS program_options REQUIRED)
include_directories(. ../include) # ${Boost_INCLUDE_DIRS})
set(SS_SERVER_HEADERS SingleThreadedServer.hpp)
set(SS_SERVER_SOURCE_FILES SingleThreadedServer.cpp ${SS_SERVER_HEADERS})
add_library(libsinglethreadedserver ${SS_SERVER_SOURCE_FILES})# ${DETECTOR_HEADERS})
add_executable(SingleThreadedServer SingleThreadedServerMain.cpp)
target_compile_features(libsinglethreadedserver PUBLIC cxx_strong_enums cxx_thread_local)
target_link_libraries(libsinglethreadedserver ${CMAKE_THREAD_LIBS_INIT})# m ${Boost_LIBRARIES})
target_link_libraries(SingleThreadedServer libsinglethreadedserver)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

################################################################################
####                           TEST                                         ####
################################################################################
enable_testing()
#set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)

#project(detector_unit_test)
#find_package(GTest)
#include_directories(include test ${GTEST_INCLUDE_DIRS})
#set(UNIT_TEST_FILES  test/detector_unit_test.cpp) #${DETECTOR_HEADERS}
#find_package ( Threads REQUIRED GTest REQUIRED)
#add_executable(detector_unit_test  ${UNIT_TEST_FILES})
#target_link_libraries ( detector_unit_test m gtest gtest_main ${CMAKE_THREAD_LIBS_INIT} )
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

