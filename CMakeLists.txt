cmake_minimum_required(VERSION 3.24)
project(Kasumi_Backends VERSION 2.0.0 DESCRIPTION "Multi-backend Graphics Api" LANGUAGES CXX)

option(OpenGL "Enable OpenGL backend" ON)
option(Vulkan "Enable Vulkan backend" OFF)
option(Metal "Enable Metal backend" OFF)
option(DirectX "Enable DirectX 11 backend" OFF)

if (NOT TARGET HinaPE_Common)
    set(KASUMI_COMMON_DIR "../common")
    add_subdirectory(${KASUMI_COMMON_DIR} HinaPE_Common)
endif ()

if (OpenGL)
    file(GLOB OpenGL_HEADER
            camera.h
            framebuffer.h
            light.h
            mesh.h
            model.h
            platform.h
            pose.h
            shader.h
            texture.h
            api.h
            timer.h

            objects/object3D.h
            objects/sphere.h
            objects/cube.h
            objects/plane.h
            objects/arrow.h

            deps/imgui/imgui.h
            deps/imgui/imconfig.h
            deps/imgui/imgui_internal.h
            deps/imgui/imstb_rectpack.h
            deps/imgui/imstb_textedit.h
            deps/imgui/imstb_truetype.h
            deps/imgui/implot.h
            deps/imgui/implot_internal.h
            deps/imgui/GraphEditor.h
            deps/imgui/ImCurveEdit.h
            deps/imgui/ImGradient.h
            deps/imgui/ImGuizmo.h
            deps/imgui/ImSequencer.h
            deps/imgui/ImZoomSlider.h
            deps/imgui/backends/imgui_impl_opengl3.h
            deps/imgui/backends/imgui_impl_glfw.h
            )
    file(GLOB OPENGL_IMPL
            OpenGL/camera.cpp
            OpenGL/framebuffer.cpp
            OpenGL/light.cpp
            OpenGL/mesh.cpp
            OpenGL/model.cpp
            OpenGL/platform.cpp
            OpenGL/shader.cpp
            OpenGL/texture.cpp
            OpenGL/timer.cpp

            objects/object3D.cpp
            objects/sphere.cpp
            objects/cube.cpp
            objects/plane.cpp
            objects/arrow.cpp

            deps/imgui/imgui.cpp
            deps/imgui/imgui_draw.cpp
            deps/imgui/imgui_widgets.cpp
            deps/imgui/imgui_tables.cpp
            deps/imgui/implot.cpp
            deps/imgui/implot_items.cpp
            deps/imgui/GraphEditor.cpp
            deps/imgui/ImCurveEdit.cpp
            deps/imgui/ImGradient.cpp
            deps/imgui/ImGuizmo.cpp
            deps/imgui/ImSequencer.cpp
            deps/imgui/backends/imgui_impl_opengl3.cpp
            deps/imgui/backends/imgui_impl_glfw.cpp
            )
    if (NOT TARGET glad)
        add_subdirectory(deps/glad)
        target_include_directories(glad PUBLIC deps)
    endif ()
    if (NOT TARGET glfw)
        add_subdirectory(deps/glfw3)
    endif ()
    if (NOT TARGET nfd)
        add_subdirectory(deps/nfd)
    endif ()
    if (NOT TARGET assimp)
        set(ASSIMP_BUILD_COLLADA_IMPORTER TRUE)
        set(ASSIMP_BUILD_OBJ_IMPORTER TRUE)
        set(ASSIMP_BUILD_PLY_IMPORTER TRUE)
        set(ASSIMP_BUILD_FBX_IMPORTER TRUE)
        set(ASSIMP_BUILD_GLTF_IMPORTER TRUE)
        set(ASSIMP_BUILD_3DS_IMPORTER TRUE)
        set(ASSIMP_BUILD_STL_IMPORTER TRUE)
        set(ASSIMP_BUILD_MMD_IMPORTER TRUE)
        set(ASSIMP_BUILD_BLEND_IMPORTER TRUE)
        set(ASSIMP_BUILD_COLLADA_EXPORTER TRUE)
        add_subdirectory(deps/assimp)
        if (APPLE)
            target_compile_options(assimp PUBLIC -Wno-error)
        endif ()
    endif ()
    add_library(Kasumi_Backends ${OpenGL_HEADER} ${Imgui_OpenGL})
    set_target_properties(Kasumi_Backends PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS ON)
    target_include_directories(Kasumi_Backends PUBLIC ${KASUMI_COMMON_DIR} deps deps/assimp/include deps/glfw3/include deps/imgui deps/stb)
    target_include_directories(Kasumi_Backends PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../)
    target_link_libraries(Kasumi_Backends PUBLIC assimp)
    target_link_libraries(Kasumi_Backends PUBLIC glad)
    target_link_libraries(Kasumi_Backends PUBLIC glfw)
    target_link_libraries(Kasumi_Backends PUBLIC nfd)
    target_link_libraries(Kasumi_Backends PUBLIC HinaPE_Common)
    target_compile_definitions(
            Kasumi_Backends
            PUBLIC
            BackendsSceneDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/scene/"
            BackendsModelDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/model/"
            BackendsShaderDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/shader/glsl/"
            BackendsTextureDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/texture/"
    )
endif ()

if (Vulkan)
    find_package(Vulkan REQUIRED)
    file(GLOB Vulkan_IMPL
            Vulkan/shader.cpp Vulkan/shader.h
            Vulkan/platform.cpp Vulkan/platform.h
            #            Vulkan/mesh.cpp
            #            Vulkan/texture.cpp
            #            Vulkan/framebuffer.cpp
            #            Vulkan/model.cpp
            #            Vulkan/camera.cpp
            #            Vulkan/instance.cpp
            )
    set(Imgui_Vulkan
            deps/imgui/imgui.h
            deps/imgui/imgui.cpp
            deps/imgui/imgui_draw.cpp
            deps/imgui/imgui_widgets.cpp
            deps/imgui/imgui_tables.cpp
            deps/imgui/imconfig.h
            deps/imgui/imgui_internal.h
            deps/imgui/imstb_rectpack.h
            deps/imgui/imstb_textedit.h
            deps/imgui/imstb_truetype.h
            deps/imgui/implot.h
            deps/imgui/implot.cpp
            deps/imgui/implot_items.cpp
            deps/imgui/implot_internal.h
            deps/imgui/backends/imgui_impl_vulkan.cpp
            deps/imgui/backends/imgui_impl_vulkan.h
            deps/imgui/backends/imgui_impl_glfw.cpp
            deps/imgui/backends/imgui_impl_glfw.h
            )
    add_executable(VulkanApp Vulkan/main.cpp ${Vulkan_IMPL} ${Imgui_Vulkan})
    set_target_properties(VulkanApp PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS ON)
    target_include_directories(VulkanApp PUBLIC ${KASUMI_COMMON_DIR} deps deps/assimp/include deps/glfw3/include deps/imgui deps/stb)
    target_include_directories(VulkanApp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
    if (NOT TARGET glfw)
        add_subdirectory(deps/glfw3)
    endif ()
    target_link_libraries(VulkanApp PUBLIC glfw Vulkan::Vulkan)
    target_link_libraries(VulkanApp PUBLIC HinaPE_Common)
    target_compile_definitions(
            VulkanApp
            PUBLIC
            -DBackendsSceneDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/scene/"
            -DBackendsModelDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/model/"
            -DBackendsShaderDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/shader/glsl/"
            -DBackendsTextureDir="${CMAKE_CURRENT_SOURCE_DIR}/assets/texture/"
    )
endif ()
