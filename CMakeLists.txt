cmake_minimum_required(VERSION 3.14)
project(semaphore)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if (NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
endif()
include(FetchContent)
FetchContent_Declare(
  googletest
  # The next release v1.17.0 requires at least C++17, this release doesn't
  URL https://github.com/google/googletest/archive/refs/tags/v1.16.0.tar.gz
)
FetchContent_Declare(
  googlebenchmark
  # The next release v1.9.1 requires at least C++17, this release doesn't
  URL https://github.com/google/benchmark/archive/refs/tags/v1.9.0.tar.gz
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# Disable tests added by Google Benchmark because CTest will run them otherwise
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
FetchContent_MakeAvailable(googlebenchmark)
add_library(semaphore semaphore.hpp semaphore.cpp)
add_executable(tests tests.cpp)
target_link_libraries(tests GTest::gtest_main benchmark::benchmark semaphore)
enable_testing()
add_test(NAME tests COMMAND $<TARGET_FILE:tests>)
set_tests_properties(tests PROPERTIES TIMEOUT 20)
