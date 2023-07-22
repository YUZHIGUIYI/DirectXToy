file(GLOB_RECURSE SRCFILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

add_library(Toy STATIC ${SRCFILES})

target_link_libraries(Toy PUBLIC d3d11.lib dxgi.lib dxguid.lib D3DCompiler.lib winmm.lib d2d1.lib dwrite.lib)
target_link_libraries(Toy PUBLIC glfw)
target_link_libraries(Toy PUBLIC spdlog_header_only)
target_link_libraries(Toy PUBLIC ImGui)
target_link_libraries(Toy PUBLIC ImGuizmo)
target_link_libraries(Toy PUBLIC DirectXTex)
target_link_libraries(Toy PUBLIC assimp)
target_link_libraries(Toy PUBLIC EnTT::EnTT)

target_include_directories(Toy PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")
target_include_directories(Toy PUBLIC "${STB_INCLUDE_DIR}")
target_include_directories(Toy PUBLIC "${ICON_FONT_DIR}")

target_precompile_headers(Toy PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include/pch.h")

set_target_properties(Toy PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/lib")
set_target_properties(Toy PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/lib")