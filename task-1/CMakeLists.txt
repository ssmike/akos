cmake_minimum_required(VERSION 2.8)

project(getss)

#add_executable(client client.c)

add_library(getss SHARED getss.c)

#target_link_libraries(client getss)
