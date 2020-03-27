# Copyright Barry Revzin 2020
# (See accompanying LICENSE)

cmake_minimum_required(VERSION 3.6)
project(span-ext CXX)

add_library(span-ext INTERFACE)
target_include_directories(span-ext INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# optionally include range-v3 (necessary for libc++ which
# does not yet have concepts/ranges)
set(SPAN_EXT_RANGE_V3_PATH "" CACHE STRING "path to range-v3")
if(SPAN_EXT_RANGE_V3_PATH)
    target_include_directories(span-ext INTERFACE ${SPAN_EXT_RANGE_V3_PATH}/include)
endif()

option(SPAN_EXT_LIBCXX "Use -libc++" OFF)
if(SPAN_EXT_LIBCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++ -lc++abi")
endif()

option(SPAN_EXT_TESTS "Build the tests" OFF)
if(SPAN_EXT_TESTS)
    include(CTest)
    add_subdirectory(test)
endif()
