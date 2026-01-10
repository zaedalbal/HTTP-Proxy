cmake_minimum_required(VERSION 3.31)
project(proxy)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)
set(TOML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/tomlplusplus)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    SOURCE_DIR ${TOML_DIR}
)
FetchContent_MakeAvailable(tomlplusplus)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS system thread)

file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_SOURCE_DIR}/src/*.cpp)

add_executable(proxy ${PROJECT_SOURCES})

target_include_directories(
    proxy PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)
target_link_libraries(
    proxy PRIVATE
    tomlplusplus::tomlplusplus
    ${Boost_LIBRARIES}
)