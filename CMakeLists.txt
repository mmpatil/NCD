cmake_minimum_required(VERSION 2.8.4)
project(ncd)
#set (ncd_VERSION_MAJOR 0)
#set (ncd_VERSION_MINOR 1)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set(SOURCE_FILES ping.c tracert.c)
add_executable(skel ${SOURCE_FILES})