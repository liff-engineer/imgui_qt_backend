cmake_minimum_required(VERSION 3.16)

project(imgui_qt_backend)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
    cmake_policy(SET CMP0135 NEW)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIG>")
endif()

include(CMakePrintHelpers)

if(DEFINED ENV{QTDIR})
    list(APPEND CMAKE_PREFIX_PATH $ENV{QTDIR})
    message(STATUS "使用环境变量QTDIR指向的Qt库")
    cmake_print_variables(CMAKE_PREFIX_PATH)
else()
    message(FATAL_ERROR "无法找到Qt,请配置环境变量QTDIR指向Qt所在路径")
endif()

find_package(Qt5 COMPONENTS Core Widgets REQUIRED)
include(DeployQtRuntime.cmake)

add_subdirectory(external)
add_subdirectory(src)
add_subdirectory(examples)
