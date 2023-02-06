cmake_minimum_required(VERSION 3.18)
project(KASUMI_BACKEND_UNIT_TEST VERSION 1.0.0 DESCRIPTION "HinaPE is not a Physics Engine" LANGUAGES CXX)

if (NOT TARGET HinaPE_Common)
    message(FATAL_ERROR "BACKEND: HinaPE_Common is not found")
    return()
endif ()

add_executable(Kasumi_backend_UNIT_TEST test_platform.cpp)
set_target_properties(Kasumi_backend_UNIT_TEST PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS ON)
target_link_libraries(Kasumi_backend_UNIT_TEST PRIVATE HinaPE_Common Kasumi_backend)

#enable_testing()
#target_link_libraries(Kasumi_backend_UNIT_TEST PUBLIC GTest::gtest_main)
#include(GoogleTest)
#gtest_discover_tests(Kasumi_backend_UNIT_TEST)