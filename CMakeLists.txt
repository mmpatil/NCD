cmake_minimum_required(VERSION 2.8.4)
project(detector)
#set (detector_VERSION_MAJOR 0)
#set (detector_VERSION_MINOR 1)

include_directories(include)
set(detector_HEADERS include/detector.hpp  include/packet.hpp include/ip_checksum.h
        include/bitset.h include/udp_packet.hpp include/tcp_packet.hpp
        include/icmp_packet.hpp)
set(SOURCE_FILES src/detector_main.cpp)
find_package(Threads REQUIRED )
add_executable(detector  ${SOURCE_FILES} ${detector_HEADERS})
target_link_libraries(detector ${CMAKE_THREAD_LIBS_INIT}  m)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")
#add_custom_target(ncd_main COMMAND make)
#execute_process(COMMAND cp ncd_main ${PROJECT_BINARY_DIR})
MESSAGE( STATUS "Detector headers: " ${detector_HEADERS} )

#project(detector_unit_test)
#includeinclude_directories(include)
#set(SOURCE_FILES test/detector_unit_test.cpp include/detector.hpp include/ip_checksum.h include/bitset.h)
#find_package ( Threads REQUIRED )
#add_executable(detector  ${SOURCE_FILES})
#target_link_libraries ( detector ${CMAKE_THREAD_LIBS_INIT}  m)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")


