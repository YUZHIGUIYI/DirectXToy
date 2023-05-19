# TODO: make it work in Ninja
# Compile HLSL-format file to CSO-format file
# Only work in MSBuild, Ninja does not work
file(GLOB_RECURSE HLSL_FILES CONFIGURE_DEPENDS "${PROJECT_SOURCE_DIR}/data/shaders/*.hlsl" "${PROJECT_SOURCE_DIR}/data/shaders/*.hlsli")
foreach(HLSL_FILE ${HLSL_FILES})
    get_filename_component(HLSL_FDIR ${HLSL_FILE} DIRECTORY)
    get_filename_component(HLSL_FBASENAME_WE ${HLSL_FILE} NAME_WE)

    string(CONCAT HLSL_FNAME_WE ${HLSL_FDIR} / ${HLSL_FBASENAME_WE})
    string(LENGTH ${HLSL_FBASENAME_WE} LEN_FNAME_WE)

    math(EXPR LEN_FNAME_WE "${LEN_FNAME_WE}-2")
    string(SUBSTRING ${HLSL_FBASENAME_WE} ${LEN_FNAME_WE} 2 ENTRY_POINT)
    string(TOLOWER ${ENTRY_POINT} SHADER_TYPE)
    # Match shader type
    if ("${SHADER_TYPE}" STREQUAL "vs")
        set(SHADER_TYPE "Vertex")
    elseif("${SHADER_TYPE}" STREQUAL "hs")
        set(SHADER_TYPE "Hull")
    elseif("${SHADER_TYPE}" STREQUAL "ds")
        set(SHADER_TYPE "Domain")
    elseif("${SHADER_TYPE}" STREQUAL "gs")
        set(SHADER_TYPE "Geometry")
    elseif("${SHADER_TYPE}" STREQUAL "ps")
        set(SHADER_TYPE "Pixel")
    elseif("${SHADER_TYPE}" STREQUAL "cs")
        set(SHADER_TYPE "Compute")
    endif()
    # In HLSL-format shader file, entry point is capitalized, thus make entry point string to upper
    string(TOUPPER ${ENTRY_POINT} ENTRY_POINT)
    # Set HLSL-format shader file compilation property
    set_source_files_properties(${HLSL_FILE} PROPERTIES
            VS_SHADER_OBJECT_FILE_NAME ${HLSL_FDIR}/${HLSL_FBASENAME_WE}.cso
            VS_SHADER_TYPE ${SHADER_TYPE}
            VS_SHADER_MODEL 5.0
            VS_SHADER_ENTRYPOINT ${ENTRY_POINT}
            VS_SHADER_DISABLE_OPTIMIZATIONS $<$<CONFIG:Debug>:true>
            VS_SHADER_ENABLE_DEBUG $<$<CONFIG:Debug>:true>)
endforeach()

file(GLOB_RECURSE SANDBOX_SRCFILES CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/include/*.h" "${CMAKE_CURRENT_LIST_DIR}/src/*.cpp")

add_executable(Sandbox ${SANDBOX_SRCFILES} ${HLSL_FILES})

target_link_libraries(Sandbox PUBLIC Toy)

target_include_directories(Sandbox PUBLIC "${CMAKE_CURRENT_LIST_DIR}/include")

set_target_properties(Sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG "${PROJECT_SOURCE_DIR}/bin")
set_target_properties(Sandbox PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE "${PROJECT_SOURCE_DIR}/bin")