cmake_minimum_required(VERSION 2.8)

project(MOOD2Be)

#############################################################

get_filename_component(MOOD2Be ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(EXECUTOR_PATH "${CMAKE_CURRENT_SOURCE_DIR}/Executor")
add_subdirectory(${EXECUTOR_PATH})
