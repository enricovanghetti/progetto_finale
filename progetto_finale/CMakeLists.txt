cmake_minimum_required(VERSION 3.10)

project(HomeSystem VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(SRC_DIR src)
set(INCLUDE_DIR include)

set(SOURCES
    ${SRC_DIR}/main_program.cpp
    ${SRC_DIR}/device_implementation.cpp
    ${SRC_DIR}/DeviceManager.cpp
)

include_directories(${INCLUDE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -fno-omit-frame-pointer -g -O2")

add_executable(home_system ${SOURCES})

add_custom_target(run
    COMMAND ./home_system 2>&1 | tee -a log.txt
    DEPENDS home_system
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)  
