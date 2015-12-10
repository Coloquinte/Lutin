
cmake_minimum_required(VERSION 2.6)
project(LUTIN)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall -Werror -std=c++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2")

include_directories(${LUT_SOURCE_DIR}/include/public)
include_directories(${LUT_SOURCE_DIR}/include/private)

set(CPPS
  src/Lut.cc
  src/LutRefConstruction.cc
  src/LutRefManipulation.cc
  src/LutPrint.cc
)

set(TESTCPPS
  tests/BasicTest.cc
)

add_library(Lutin ${CPPS})

enable_testing()

add_executable(tests.bin ${TESTCPPS})
target_link_libraries(tests.bin Lutin)

add_test(basictests tests.bin)

