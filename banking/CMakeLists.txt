cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(banking)

add_library(account STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Account.cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_library(transaction STATIC ${CMAKE_CURRENT_SOURCE_DIR}/Transaction.cpp)
target_link_libraries(transaction account)
