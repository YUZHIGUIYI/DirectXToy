cmake_minimum_required(VERSION 3.15)

project(DirectXTex LANGUAGES CXX C)

file(GLOB_RECURSE TEX_SRC_FILES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_LIST_DIR}/DDSTextureLoader/*.h" 
    "${CMAKE_CURRENT_LIST_DIR}/DDSTextureLoader/*.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/WICTextureLoader/*.h"
    "${CMAKE_CURRENT_LIST_DIR}/WICTextureLoader/*.cpp")

add_library(DirectXTex STATIC ${TEX_SRC_FILES})

target_include_directories(DirectXTex PUBLIC ${CMAKE_CURRENT_LIST_DIR})

set_target_properties(DirectXTex PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
set_target_properties(DirectXTex PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")