cmake_minimum_required(VERSION 3.15 FATAL_ERROR)

project(DirectX11Toy LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}/bin")

include(cmake/HLSLShaders.cmake)

add_subdirectory(external/glfw)
add_subdirectory(external/spdlog)
add_subdirectory(external/imgui)
add_subdirectory(external/DirectXTex)
add_subdirectory(Toy)
add_subdirectory(Sandbox)
