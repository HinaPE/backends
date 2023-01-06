cmake_minimum_required(VERSION 3.18)
project(Kasumi_igl VERSION 0.2.0 DESCRIPTION "Kasumi: Illumine the Endless Night." LANGUAGES CXX C)

file(GLOB_RECURSE IGL_SRC *.h *.cpp *.c)
add_library(igl ${IGL_SRC})
set_target_properties(igl PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS ON)
target_include_directories(igl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(igl PUBLIC IGL_STATIC_LIBRARY)