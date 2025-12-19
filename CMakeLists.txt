cmake_minimum_required(VERSION 3.31)
project(proxy)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)

file(GLOB_RECURSE PROJECT_SOURCES src/*.cpp include/*.hpp main.cpp)

add_executable(proxy ${PROJECT_SOURCES})

target_include_directories(proxy PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(proxy PRIVATE ${Boost_LIBRARIES})