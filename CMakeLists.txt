cmake_minimum_required(VERSION 3.21)

project(EduHub
    VERSION 0.1.0
    DESCRIPTION "Offline university management system"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(EDUHUB_BUILD_TESTS "Build the Qt Test suites" ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Widgets Sql Svg Test)
qt_standard_project_setup()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -Wnon-virtual-dtor)
endif()

add_subdirectory(src)

if(EDUHUB_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
