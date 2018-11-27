cmake_minimum_required(VERSION 2.8)

project(IntralogisticExecutor)

#############################################################
find_package(BehaviorTree REQUIRED)

include_directories( ./include )

if(NOT CMAKE_VERSION VERSION_LESS 3.1)
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CONFIG_PATH ${CMAKE_MODULE_PATH}  "${CMAKE_CURRENT_LIST_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CONFIG_PATH}")

find_package(ZMQ REQUIRED)

#############################################################


add_executable(${PROJECT_NAME}
    src/skill_interface.cpp
    src/skill_parser.cpp
    src/executor.cpp
    )

target_link_libraries(${PROJECT_NAME} behaviortree_cpp dl pthread zmq)




