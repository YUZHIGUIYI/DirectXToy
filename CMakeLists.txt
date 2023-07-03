cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(DirectX11Toy LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")

# For assimp library
set(ASSIMP_BUILD_ZLIB ON)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)

# For stb
set(STB_INCLUDE_DIR "${CMAKE_CURRENT_LIST_DIR}/external/stb" CACHE PATH "stb header file directory" FORCE)

# For IconFontCppHeaders
set(ICON_FONT_DIR "${CMAKE_CURRENT_LIST_DIR}/external/IconFontCppHeaders" CACHE PATH "Icon font cpp headers directory" FORCE)

# Copy ImGui setting file
file(COPY "${CMAKE_CURRENT_LIST_DIR}/data/settings/imgui.ini" DESTINATION "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

include(cmake/HLSLShaders.cmake)

add_subdirectory(external/assimp)
add_subdirectory(external/glfw)
add_subdirectory(external/spdlog)
add_subdirectory(external/imgui)
add_subdirectory(external/DirectXTex)
add_subdirectory(external/entt)
add_subdirectory(Toy)
add_subdirectory(Sandbox)
