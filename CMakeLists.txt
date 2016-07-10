cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(wkserial)

if(CMAKE_COMPILER_IS_GNUCC)
   list(APPEND CMAKE_CXX_FLAGS "-fPIC" )
   list(APPEND CMAKE_LD_FLAGS "-fPIC" )
endif()

add_subdirectory(source)

