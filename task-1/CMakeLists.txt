cmake_minimum_required(VERSION 2.8)

project(getss)

add_executable(getss-client client.c)

add_library(getss STATIC getss.c)

target_link_libraries(getss-client getss)
