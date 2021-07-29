cmake_minimum_required(VERSION 3.1)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
find_package(GTest REQUIRED)

## Set our project name
project(cxxlisp)

set(srcs
  value.cpp
  lexer.cpp
  parser.cpp
  vm.cpp
  )

set(test_srcs
  value_test.cpp
  lexer_test.cpp
  parser_test.cpp
  vm.cpp)
  
## cxxlisp
add_executable(cxxlisp main.cpp ${srcs})
target_link_libraries(cxxlisp)

## testcxxlisp
add_executable(testcxxlisp ${srcs} ${test_srcs})
target_link_libraries(testcxxlisp
  PRIVATE
  GTest::GTest
  GTest::Main)
