cmake_minimum_required(VERSION 2.8)

project(IntralogisticExecutor)

find_package(BehaviorTree REQUIRED)

include_directories( ./include )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME}  executor.cpp )




