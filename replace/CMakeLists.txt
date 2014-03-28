cmake_minimum_required(VERSION 2.8)

project(replace)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../task-1
    ${CMAKE_CURRENT_BINARY_DIR}/getss)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../task-1)

add_executable(main main.c)

target_link_libraries(main getss)
