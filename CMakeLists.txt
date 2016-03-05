cmake_minimum_required(VERSION 2.8.4)
project(detector)
#set (detector_VERSION_MAJOR 0)
#set (detector_VERSION_MINOR 1)

include_directories(include)
set(DETECTOR_HEADERS include/detector.hpp  include/packet.hpp include/ip_checksum.h
        include/simple_bitset.h include/udp_packet.hpp include/tcp_packet.hpp
        include/icmp_packet.hpp include/ip_tcp_packet.hpp include/ip_udp_packet.hpp
        include/ip_icmp_packet.hpp include/udp_detector.hpp include/tcp_detector.hpp)
set(DETECTOR_SOURCE_FILES src/detector_main.cpp)
find_package(Threads REQUIRED )
add_executable(detector  ${DETECTOR_SOURCE_FILES} ${DETECTOR_HEADERS})
target_link_libraries(detector ${CMAKE_THREAD_LIBS_INIT}  m)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
#add_custom_target(ncd_main COMMAND make)
#execute_process(COMMAND cp ncd_main ${PROJECT_BINARY_DIR})
MESSAGE( STATUS "Detector headers: " ${detector_HEADERS} )


project(ncd)
#set (ncd_VERSION_MAJOR 0)
#set (ncd_VERSION_MINOR 1)

#include_directories(BEFORE include)
set(NCD_SOURCE_FILES src/ncd.c src/ncd_main.c include/ncd_global.h include/simple_bitset.h)
find_package ( Threads REQUIRED )
add_executable(ncd_main  ${NCD_SOURCE_FILES})
target_link_libraries ( ncd_main ${CMAKE_THREAD_LIBS_INIT}  m)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")


################################################################################
####                           TEST                                         ####
################################################################################
enable_testing()
#set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)



project(detector_unit_test)
include_directories(include ${GTEST_INCLUDE_DIRS})
set(UNIT_TEST_FILES ${DETECTOR_HEADERS} test/detector_unit_test.cpp test/detector_unit_test.hpp)
find_package ( Threads REQUIRED GTest REQUIRED)
add_executable(detector_unit_test  ${UNIT_TEST_FILES})
target_link_libraries ( detector_unit_test ${CMAKE_THREAD_LIBS_INIT} m gtest gtest_main)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")



project(ncd_unit_test)
include_directories(include ${GTEST_INCLUDE_DIRS})
set(UNIT_TEST_FILES ${detector_HEADERS} test/detector_unit_test.cpp test/detector_unit_test.hpp)
find_package ( Threads REQUIRED GTest REQUIRED)
add_executable(ncd_unit_test  ${UNIT_TEST_FILES})
target_link_libraries ( detector_unit_test ${CMAKE_THREAD_LIBS_INIT} m gtest gtest_main)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
