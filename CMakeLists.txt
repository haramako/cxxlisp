cmake_minimum_required(VERSION 3.1)
enable_language(CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-O2 -Wall -Wextra -Wno-unused-parameter -Wno-sequence-point -Wno-unused-private-field")
set(CMAKE_LD_FLAGS "-lgc")
find_package(GTest REQUIRED)

option(gc_enable "Use Boehm GC" ON)

## Switch GC implementation.
if( gc_enable )
  # Use libgc
  find_library(gc NAMES libgc.a)
  set(gclib gc)
  add_compile_definitions(CXXLISP_GC_ENABLED)
else()
  # No GC.
  set(gclib)
endif()

## Set our project name
project(cxxlisp)

set(srcs
  errors.cpp
  lib_core.cpp
  lib_number.cpp
  lib_list.cpp
  lib_string.cpp
  parser.cpp
  pretty_print.cpp
  util.cpp
  value.cpp
  vm.cpp
  )

set(test_srcs
  value_test.cpp
  parser_test.cpp
  vm_test.cpp)
  
## cxxlisp
add_executable(cxxlisp main.cpp ${srcs})
target_link_libraries(cxxlisp PRIVATE ${gclib})

## testcxxlisp
add_executable(testcxxlisp ${srcs} ${test_srcs})
target_link_libraries(testcxxlisp
  PRIVATE
  GTest::GTest
  GTest::Main
  ${gclib}
  )


## target 'test'
add_custom_target(test COMMAND ./testcxxlisp )
add_dependencies(test testcxxlisp)
   


