cmake_minimum_required(VERSION 3.31)
project(proxy)

option(BUILD_PROXY_SERVER "Build the proxy server" ON)
option(BUILD_ADMIN_PANEL "Build the admin panel" ON)
option(BUILD_TOOLS "Build the tools" ON)

include(FetchContent)
set(TOML_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/tomlplusplus)
FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
    SOURCE_DIR ${TOML_DIR}
)
FetchContent_MakeAvailable(tomlplusplus)

if(BUILD_PROXY_SERVER)
    add_subdirectory(proxy_server)
endif()

if(BUILD_TOOLS)
    add_subdirectory(tools)
endif()

if(BUILD_ADMIN_PANEL)
    add_subdirectory(admin_panel)
endif()