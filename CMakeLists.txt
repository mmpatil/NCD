cmake_minimum_required(VERSION 2.8.12)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

project(detector)
#set (detector_VERSION_MAJOR 0)
#set (detector_VERSION_MINOR 1)

#set(Boost_USE_STATIC_LIBS ON)
find_package(Threads REQUIRED)
find_package(Boost 1.6 COMPONENTS program_options REQUIRED)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include ${Boost_INCLUDE_DIRS})
set(DETECTOR_HEADERS include/detector.hpp  include/packet.hpp include/ip_checksum.h
        include/simple_bitset.h include/udp_packet.hpp include/tcp_packet.hpp
        include/icmp_packet.hpp include/ip_tcp_packet.hpp include/ip_udp_packet.hpp
        include/ip_icmp_packet.hpp include/udp_detector.hpp include/tcp_detector.hpp include/co_op_data.hpp)
set(DETECTOR_SOURCE_FILES src/detector_cfg.cpp)
add_executable(detector  ${DETECTOR_SOURCE_FILES} ${DETECTOR_HEADERS})
target_compile_features(detector PUBLIC cxx_strong_enums cxx_thread_local)
target_link_libraries(detector ${CMAKE_THREAD_LIBS_INIT} m ${Boost_LIBRARIES})
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
#add_custom_target(ncd_main COMMAND make)
#execute_process(COMMAND cp ncd_main ${PROJECT_BINARY_DIR})
MESSAGE(STATUS "Detector headers: " ${detector_HEADERS} )


add_subdirectory(ncd EXCLUDE_FROM_ALL)
add_subdirectory(co_op_udp_server)
################################################################################
####                           TEST                                         ####
################################################################################
enable_testing()
#set(PROJECT_TEST_NAME ${PROJECT_NAME_STR}_test)

project(detector_unit_test)
find_package(GTest)
include_directories(include test ${GTEST_INCLUDE_DIRS})
set(UNIT_TEST_FILES  test/detector_unit_test.cpp) #${DETECTOR_HEADERS}
find_package ( Threads REQUIRED GTest REQUIRED)
add_executable(detector_unit_test  ${UNIT_TEST_FILES})
target_link_libraries ( detector_unit_test m gtest gtest_main ${CMAKE_THREAD_LIBS_INIT} )
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")

