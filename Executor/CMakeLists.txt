cmake_minimum_required(VERSION 2.8)

project(Executor)

#############################################################

get_filename_component(MOOD2Be ${CMAKE_CURRENT_SOURCE_DIR} DIRECTORY)
set(BEHAVIOR_TREE_PATH "${MOOD2Be}/BehaviorTree.CPP")
set(BEHAVIOR_TREE_INCLUDE_PATH "${BEHAVIOR_TREE_PATH}/include")
set(BEHAVIOR_TREE_BUILD "${CMAKE_CURRENT_BINARY_DIR}/BehaviorTree.CPP")

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${MOOD2Be}/.git")
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Submodule update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                        WORKING_DIRECTORY ${MOOD2Be}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endif()

if(NOT EXISTS "${BEHAVIOR_TREE_INCLUDE_PATH}")
    message(FATAL_ERROR "The submodules were not downloaded.")
endif()

add_subdirectory(${BEHAVIOR_TREE_PATH} ${BEHAVIOR_TREE_BUILD})
include_directories(${BEHAVIOR_TREE_PATH}/include)

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
  src/check_positive_condition.cpp
  src/skill_action.cpp
  src/skill_parser.cpp
  src/variant_action.cpp
  src/execute_path_action.cpp
  src/executor.cpp
  src/txt_to_waypoints_action.cpp
  )

target_link_libraries(${PROJECT_NAME} behaviortree_cpp_v3 dl pthread zmq)

add_executable( zmq_dummy_server zmq_server.cpp )
target_link_libraries(zmq_dummy_server pthread zmq)