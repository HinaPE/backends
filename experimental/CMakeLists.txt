cmake_minimum_required(VERSION 3.18)
project(Pathtracer VERSION 1.0.0 DESCRIPTION "Kasumi Pathtracer" LANGUAGES CXX)

find_package(Threads REQUIRED)

#set_property(GLOBAL PROPERTY USE_FOLDERS ON)
#
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX=1")
#
#if (WIN32)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_WINDOWS=1")
#    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
#elseif (APPLE)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_MAC=1")
#    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -framework Cocoa -framework QuartzCore")
#    set(CMAKE_XCODE_GENERATE_SCHEME ON)
#    set(XCODE_SCHEME_ENVIRONMENT ON)
#elseif (UNIX)
#    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPLATFORM_LINUX=1")
#endif ()

add_subdirectory(src)
add_subdirectory(imgui)
add_subdirectory(imguizmo)
target_include_directories(imgui PUBLIC imgui imguizmo ../deps ../deps/glfw3/include)
target_include_directories(imguizmo PUBLIC imgui imguizmo ../deps ../deps/glfw3/include)
target_link_libraries(imguizmo PUBLIC imgui)

# *.cpp *.h */*.h */*.cpp
file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.*" "${CMAKE_CURRENT_SOURCE_DIR}/shaders/*/*.*")
foreach(file ${files})
    SET(ASSETS
            ${ASSETS}
            ${file}
            )
endforeach()
#source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}" PREFIX "Resources" FILES ${ASSETS})

add_executable(PathTracer
        src/Main.cpp
        ${ASSETS}
        )

set_target_properties(PathTracer PROPERTIES CXX_STANDARD 17 CXX_EXTENSIONS ON)

target_link_libraries(PathTracer PUBLIC Core glfw glad imgui imguizmo Threads::Threads ${OPENGL_LIBRARY})

add_custom_command(TARGET PathTracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets $<TARGET_FILE_DIR:PathTracer>/assets/
        )

add_custom_command(TARGET PathTracer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders $<TARGET_FILE_DIR:PathTracer>/shaders/
        )