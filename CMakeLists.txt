cmake_minimum_required(VERSION 3.26)
project(AletherEngine)

set(CMAKE_CXX_STANDARD 23)
configure_file(config.in ${CMAKE_CURRENT_SOURCE_DIR}/src/config.h)

## dependency
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL ON CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/assimp)

add_subdirectory(external/glfw)
add_subdirectory(external/glm)

file(GLOB_RECURSE ENGINE_SOURCE ./src/*.cpp external/glad/src/glad.c)
file(GLOB_RECURSE ENGINE_HEADER ./src/*.h)

## stb needs to be added manualy to executable
#file(GLOB_RECURSE STB_SOURCE_HEADER ./external/stb/stb_image.h)

## Register AletherEngine exe
add_executable(AletherEngine
        cmd/editor/main.cpp
        ${ENGINE_HEADER}
        ${ENGINE_SOURCE}
)
target_include_directories(AletherEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        external/glm
        external/stb
        external/glad/include
        external/assimp/include
        ${CMAKE_BINARY_DIR}/external/assimp/include
)
target_link_libraries(AletherEngine glfw glm::glm assimp::assimp)

## Register RaymarchSdfCpu exe
add_executable(RaymarcherSdfCpu
        cmd/raymarch_sdf_cpu/main.cpp
        ${ENGINE_HEADER}
        ${ENGINE_SOURCE}
)
target_include_directories(RaymarcherSdfCpu PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        external/glm
        external/stb
        external/glad/include
        external/assimp/include
        ${CMAKE_BINARY_DIR}/external/assimp/include
)
target_link_libraries(RaymarcherSdfCpu glfw glm::glm assimp::assimp)

## Register RaymarchSdf exe
add_executable(RaymarchSdf
        cmd/raymarch_sdf/main.cpp
        ${ENGINE_HEADER}
        ${ENGINE_SOURCE}
)
target_include_directories(RaymarchSdf PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
        external/glm
        external/stb
        external/glad/include
        external/assimp/include
        ${CMAKE_BINARY_DIR}/external/assimp/include
)
target_link_libraries(RaymarchSdf glfw glm::glm assimp::assimp)