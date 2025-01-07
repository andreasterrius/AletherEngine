cmake_minimum_required(VERSION 3.22.1)

project(AletherEngine)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_STANDARD 20)
configure_file(config.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

## dependency
set(BUILD_SHARED_LIBS ON)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL ON)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)
add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/entt)

## according to imgui guide, it is recommended that you follow those steps
## and not attempt to build Dear ImGui as a static or shared library!
# Add source files for Dear ImGui
set(IMGUI_SOURCE
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_demo.cpp
        external/imgui/backends/imgui_impl_glfw.cpp  # Use GLFW backend
        external/imgui/backends/imgui_impl_opengl3.cpp  # Use OpenGL3 backend
)

# Add header files for Dear ImGui (optional for IDEs to index them)
set(IMGUI_HEADER
        external/imgui/imgui.h
        external/imgui/imgui_internal.h
        external/imgui/imgui_demo.cpp
        external/imgui/imconfig.h
        external/imgui/imstb_rectpack.h
        external/imgui/imstb_textedit.h
        external/imgui/imstb_truetype.h
        external/imgui/backends/imgui_impl_glfw.h
        external/imgui/backends/imgui_impl_opengl3.h
)

#file(GLOB_RECURSE ENGINE_SOURCE ./src/*.cpp external/glad/src/glad.c)
file(GLOB_RECURSE ENGINE_SOURCE ./src/*.cpp external/glad/src/glad.c)
file(GLOB_RECURSE ENGINE_HEADER ./src/*.h)

## stb needs to be added manualy to executable
#file(GLOB_RECURSE STB_SOURCE_HEADER ./external/stb/stb_image.h)

function(create_exe name folder_name)
    file(GLOB_RECURSE ${folder_name}_SOURCE ./cmd/${folder_name}/*.cpp)
    file(GLOB_RECURSE ${folder_name}_HEADER ./cmd/${folder_name}/*.h)
    add_executable(${name}
            ${${folder_name}_SOURCE}
            ${${folder_name}_HEADER}
            ${ENGINE_HEADER}
            ${ENGINE_SOURCE}
            ${IMGUI_SOURCE}
            ${IMGUI_HEADER}
    )
    target_include_directories(${name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            external/glm
            external/stb
            external/glad/include
            external/assimp/include
            external/glfw/include
            ${CMAKE_BINARY_DIR}/external/assimp/includea
            external/imgui
            external/imgui/backends
    )
    target_link_libraries(${name} glfw glm::glm assimp::assimp EnTT::EnTT)
endfunction()

create_exe(AletherEngine editor)
create_exe(RaymarchSdfCpu raymarch_sdf_cpu)
create_exe(RaymarchSdf raymarch_sdf)
create_exe(SdfGenerator sdf_generator_gpu)
create_exe(Scene01 scene_01)
create_exe(HelloImgui hello_imgui)