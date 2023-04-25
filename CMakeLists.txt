cmake_minimum_required (VERSION 3.23)
project (ADRefine )

set (CMAKE_CXX_STANDARD 20)


include (external/utap.cmake)

include(CTest)

add_subdirectory (libs)
add_subdirectory (bin)
