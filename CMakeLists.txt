cmake_minimum_required(VERSION 3.30.0)

project(AletherEngine)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_STANDARD 20)
configure_file(config.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")

## Modules support
if (MSVC)
    add_compile_options(/experimental:module)
endif ()
set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "TRUE")

## dependency
set(BUILD_SHARED_LIBS ON)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL ON)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)
add_subdirectory(external/glfw)
add_subdirectory(external/json)
set(ZSTD_BUILD_SHARED ON)
add_subdirectory(external/zstd/build/cmake)

option(SPDLOG_BUILD_SHARED "Build spdlog as shared library" ON)
add_subdirectory(external/spdlog)

set(BUILD_SHARED_LIBS OFF)
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
file(GLOB_RECURSE ENGINE_CPP_MODULES ./src/*.cppm)

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
    target_sources(${name}
            PRIVATE
            FILE_SET CXX_MODULES TYPE CXX_MODULES FILES ${ENGINE_CPP_MODULES}
    )
    target_include_directories(${name} SYSTEM PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            external/glm
            external/stb
            external/glad/include
            external/assimp/include
            external/glfw/include
            ${CMAKE_BINARY_DIR}/external/assimp/include
            external/imgui
            external/imgui/backends
            external/zstd/lib
            external/json/include
    )
    target_link_libraries(${name} glfw glm::glm assimp::assimp EnTT::EnTT spdlog::spdlog nlohmann_json::nlohmann_json)
    target_compile_definitions(${name} PUBLIC SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_TRACE GLM_ENABLE_EXPERIMENTAL)
endfunction()

create_exe(RaymarchSdfCpu raymarch_sdf_cpu)
create_exe(RaymarchSdf raymarch_sdf)
create_exe(SdfGenerator sdf_generator_gpu)
create_exe(BasicRenderer basic_renderer)
create_exe(HelloImgui hello_imgui)
create_exe(Editor2 editor2)
create_exe(SdfGeneratorV2 sdf_generator_gpu_v2)
create_exe(MeshDistanceField mesh_distance_field_tutorial)
create_exe(DeferredRenderer deferred_renderer)