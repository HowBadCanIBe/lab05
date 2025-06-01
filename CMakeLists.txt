cmake_minimum_required(VERSION 3.10)
project(BankingProject)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")

add_subdirectory(third-party/gtest)
add_subdirectory(banking)

add_executable(RunTests tests/test.cpp)
target_link_libraries(RunTests PRIVATE gtest gtest_main gmock_main banking)
target_include_directories(RunTests PRIVATE banking)

enable_testing()
add_test(NAME AllTests COMMAND RunTests)
