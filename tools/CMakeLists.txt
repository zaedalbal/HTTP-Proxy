cmake_minimum_required(VERSION 3.31)
project(tools)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenSSL REQUIRED)

add_executable(admin_panel_auth_data_editor ${CMAKE_CURRENT_SOURCE_DIR}/src/admin_panel_auth_data_editor.cpp)

target_link_libraries(admin_panel_auth_data_editor
PRIVATE
OpenSSL::SSL OpenSSL::Crypto
tomlplusplus::tomlplusplus)
