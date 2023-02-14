cmake_minimum_required(VERSION 3.18)
project(BackendsTutorials VERSION 2.0.0 DESCRIPTION "Kasumi: illumine the endless night" LANGUAGES CXX)

if (NOT TARGET HinaPE_Common)
    message(FATAL_ERROR "BACKEND: HinaPE_Common is not found")
    return()
endif ()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

add_executable(backends_00platform platform.cpp)
set_target_properties(backends_00platform PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS ON)
target_link_libraries(backends_00platform PRIVATE Kasumi_backend)