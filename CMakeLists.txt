cmake_minimum_required(VERSION 3.1)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra -Wno-unused-parameter -Wno-sequence-point")
set(CMAKE_LD_FLAGS "-lgc")
find_package(GTest REQUIRED)
find_library(gc NAMES libgc.a)

## Set our project name
project(cxxlisp)

set(srcs
  errors.cpp
  func.cpp
  value.cpp
  parser.cpp
  pretty_print.cpp
  util.cpp
  vm.cpp
  )

set(test_srcs
  value_test.cpp
  parser_test.cpp
  vm_test.cpp)
  
## cxxlisp
add_executable(cxxlisp main.cpp ${srcs})
target_link_libraries(cxxlisp PRIVATE gc)

## testcxxlisp
add_executable(testcxxlisp ${srcs} ${test_srcs})
target_link_libraries(testcxxlisp
  PRIVATE
  GTest::GTest
  GTest::Main
  gc)


## target 'test'
add_custom_target(test COMMAND ./testcxxlisp )
add_dependencies(test testcxxlisp)
   
