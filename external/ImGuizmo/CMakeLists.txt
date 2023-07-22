file(GLOB IMGUIZMO_SRC_FILES "${CMAKE_CURRENT_LIST_DIR}/*.h" "${CMAKE_CURRENT_LIST_DIR}/*.cpp")

add_library(ImGuizmo STATIC ${IMGUIZMO_SRC_FILES})

target_include_directories(ImGuizmo PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# User must link imgui static library, see https://github.com/YUZHIGUIYI/imgui.git, the docking branch
target_link_libraries(ImGuizmo PUBLIC ImGui)