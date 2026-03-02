cmake_minimum_required(VERSION 3.25)
project(proxy_server)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost REQUIRED COMPONENTS log log_setup filesystem system thread)
find_package(OpenSSL REQUIRED)

file(GLOB_RECURSE PROJECT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)

add_executable(proxy ${PROJECT_SOURCES})

target_include_directories(
    proxy PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/include
)

target_link_libraries(
    proxy PRIVATE
    tomlplusplus::tomlplusplus
    ${Boost_LIBRARIES}
    OpenSSL::Crypto
    OpenSSL::SSL
)

enable_testing()
find_package(GTest REQUIRED)
include(GoogleTest)
file(GLOB_RECURSE SRC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
list(FILTER SRC_SOURCES EXCLUDE REGEX ".*/main.cpp$")
file(GLOB_RECURSE TEST_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/tests/unit_tests/*.cpp)
add_executable(tests ${SRC_SOURCES} ${TEST_SOURCES})

#!!! НЕ ЗАБЫВАТЬ ДОБАВЛЯТЬ ИСХОДНИКИ И В ТЕСТЫ ТОЖЕ!!!

target_include_directories(
    tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/api/include
)
target_link_libraries(tests GTest::gtest_main tomlplusplus::tomlplusplus ${Boost_LIBRARIES} OpenSSL::Crypto OpenSSL::SSL)
gtest_discover_tests(tests)