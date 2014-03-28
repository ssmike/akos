cmake_minimum_required(VERSION 2.8)

project(argparse)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../task-1
    ${CMAKE_CURRENT_BINARY_DIR}/getss)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../task-1)

add_executable(client client.c)

add_library(argparse SHARED argparse.c)

target_link_libraries(client getss argparse)
