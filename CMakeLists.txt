cmake_minimum_required(VERSION 3.22.1)
  
project(AletherEngine)

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
set(CMAKE_CXX_STANDARD 20)
configure_file(config.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

## dependency
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

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
    )
    target_include_directories(${name} PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            external/glm
            external/stb
            external/glad/include
            external/assimp/include
            external/glfw/include
            ${CMAKE_BINARY_DIR}/external/assimp/include
    )
    target_link_libraries(${name} glfw glm::glm assimp::assimp)
endfunction()

create_exe(AletherEngine editor)
create_exe(RaymarchSdfCpu raymarch_sdf_cpu)
create_exe(RaymarchSdf raymarch_sdf)
create_exe(SdfGenerator sdf_generator_gpu)
create_exe(Scene01 scene_01)
