# CMake entry point
cmake_minimum_required(VERSION 3.0)
project(ProceduralGeneration)

find_package(OpenGL REQUIRED)

# Assurez-vous que le dossier build est séparé du code source
if (CMAKE_BINARY_DIR STREQUAL CMAKE_SOURCE_DIR)
    message(FATAL_ERROR "Please select another Build Directory!")
endif()

if (CMAKE_SOURCE_DIR MATCHES " ")
    message("Your Source Directory contains spaces. This may cause issues during compilation.")
endif()

if (CMAKE_BINARY_DIR MATCHES " ")
    message("Your Build Directory contains spaces. This may cause issues during compilation.")
endif()

# Compilation des dépendances externes
add_subdirectory(external)

# Inclure les chemins nécessaires
include_directories(
    external/glfw-3.1.2/include/
    external/glm-0.9.7.1/
    external/glew-1.13.0/include/
)

include_directories(
	${CMAKE_SOURCE_DIR}/include
	${CMAKE_SOURCE_DIR}/imgui
    ${CMAKE_SOURCE_DIR}/external/stb
    ${CMAKE_SOURCE_DIR}/external/nlohmann
)

# Bibliothèques nécessaires
set(ALL_LIBS
    ${OPENGL_LIBRARY}
    glfw
    GLEW_1130
)

# Définitions pour la compilation
add_definitions(
    -DTW_STATIC
    -DTW_NO_LIB_PRAGMA
    -DTW_NO_DIRECT3D
    -DGLEW_STATIC
    -D_CRT_SECURE_NO_WARNINGS
)

# Fichiers shaders
set(SHADERS
    shaders/vertex_shader.glsl
    shaders/fragment_shader.glsl
    shaders/noise_compute.glsl
    shaders/noise_fragment.glsl
    shaders/noise_vertex.glsl
)

# Fichiers ImGui
file(GLOB IMGUI_SRC imgui/*.cpp)
file(GLOB IMGUI_HDR imgui/*.h)

# Fichiers du projet
file(GLOB_RECURSE SRC_FILES src/*.cpp)
file(GLOB_RECURSE INCLUDE_FILES include/*.hpp)

# Exécutable principal
add_executable(ProceduralGeneration
    ${SRC_FILES}
    ${INCLUDE_FILES}
    ${IMGUI_SRC}
    ${IMGUI_HDR}
    ${SHADERS}
)

# Lier les bibliothèques
target_link_libraries(ProceduralGeneration
    ${ALL_LIBS}
)

# Configuration des dossiers de travail pour Xcode et Visual Studio
set_target_properties(ProceduralGeneration PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bin/")
set_target_properties(ProceduralGeneration PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/bin/")

# Copier les shaders dans le dossier de sortie après compilation
foreach(SHADER ${SHADERS})
    add_custom_command(
        TARGET ProceduralGeneration POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER}
        ${CMAKE_CURRENT_SOURCE_DIR}/bin/
    )
endforeach()

# Message en cas de succès
message(STATUS "CMake setup complete. Ready to build ProceduralGeneration!")
