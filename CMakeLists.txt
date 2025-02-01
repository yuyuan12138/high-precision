cmake_minimum_required(VERSION 3.18)
if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
    -mavx512bw
    -mavx512vl
    -mavx512f
    -mavx512cd 
    -mavx512dq 
    -msse
)


set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake;${CMAKE_MODULE_PATH}")

project(high-precision)

add_subdirectory(BigInteger)

add_executable(high-precision main.cpp)

target_link_libraries(high-precision PRIVATE BigInteger)


