cmake_minimum_required(VERSION 3.31)
project(admin_panel)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

set(CMAKE_AUTOMOC ON)

target_include_directories(proxy PRIVATE ${API_INCLUDE_DIR})

file(GLOB_RECURSE PROJECT_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp # !!!!!!!!!БЕЗ ЭТОЙ СТРОКИ ВСЕ ПАДАЕТ!!!!!!!!!
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
) 

add_executable(admin_panel ${PROJECT_SOURCES})

target_include_directories(
    admin_panel PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/include
)

target_link_libraries(admin_panel PRIVATE Qt6::Widgets Qt6::Network)