cmake_minimum_required(VERSION 3.21)

# Set your project name. This will be the name of your SKSE .dll file.
project(
    SKPEPE
    VERSION 1.0.0
    DESCRIPTION
        "SKSE plugin that allows for stagger and knockdown management through perk entries."
    LANGUAGES CXX)
set(PROJECT_FRIENDLY_NAME "Stagger and Knockdown Perk Entries")

# If `SKYRIM_FOLDER` environment variable is set then DLL is copied on to the
# data directory post build
if(DEFINED ENV{SKYRIM_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_FOLDER}/Data")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_FOLDER}/Data")
endif()

# If `SKYRIM_MODS_FOLDER` environment variable is set, then DLL is copied to the
# mod staging folder. Recommended over directly copying into the data folder.
if(DEFINED ENV{SKYRIM_MODS_FOLDER} AND IS_DIRECTORY "$ENV{SKYRIM_MODS_FOLDER}")
    set(OUTPUT_FOLDER "$ENV{SKYRIM_MODS_FOLDER}/${PROJECT_NAME}")
endif()

# Otherwise, you can set OUTPUT_FOLDER to any place you'd like :)
# set(OUTPUT_FOLDER "C:/path/to/any/folder")

set(headers include/PerkEntryPointExtenderAPI.h)

set(sources src/logger.cpp src/onhithandler.cpp src/plugin.cpp)

# Setup your SKSE plugin as an SKSE plugin!
find_package(CommonLibSSE CONFIG REQUIRED)
add_commonlibsse_plugin(
    ${PROJECT_NAME}
    AUTHOR
    "vsky1"
    SOURCES
    ${headers}
    ${sources})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_23)
target_precompile_headers(${PROJECT_NAME} PRIVATE src/PCH.h)
target_include_directories(${PROJECT_NAME} PRIVATE include)

# When your SKSE .dll is compiled, this will automatically copy the .dll into
# your mods folder. Only works if you configure DEPLOY_ROOT above (or set the
# SKYRIM_MODS_FOLDER environment variable)
if(DEFINED OUTPUT_FOLDER)
    # If you specify an <OUTPUT_FOLDER> (including via environment variables)
    # then we'll copy your mod files into Skyrim or a mod manager for you!

    # Copy the SKSE plugin .dll files into the SKSE/Plugins/ folder
    set(DLL_FOLDER "${OUTPUT_FOLDER}/SKSE/Plugins")

    message(STATUS "SKSE plugin output folder: ${DLL_FOLDER}")

    add_custom_command(
        TARGET "${PROJECT_NAME}"
        POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "${DLL_FOLDER}"
        COMMAND
            "${CMAKE_COMMAND}" -E copy_if_different
            "$<TARGET_FILE:${PROJECT_NAME}>"
            "${DLL_FOLDER}/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
        VERBATIM)

    file(TO_NATIVE_PATH "${OUTPUT_FOLDER}/../${PROJECT_NAME}_ESL/"
         _eslDstFolder)

    # This mod uses an ESL file, we create a seperate symlink to the mod folder
    # We create a symlink so that we can run the creation kit and edit it and
    # have our changes be automatically synched by our mod manager as we please.
    if(NOT EXISTS "${_eslDstFolder}")
        file(TO_NATIVE_PATH "${CMAKE_SOURCE_DIR}/mod/" _eslSrcFolder)
        execute_process(COMMAND cmd.exe /c mklink /J "${_eslDstFolder}"
                                "${_eslSrcFolder}")
    endif()
    # If you perform a "Debug" build, also copy .pdb file (for debug symbols)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_custom_command(
            TARGET "${PROJECT_NAME}"
            POST_BUILD
            COMMAND
                "${CMAKE_COMMAND}" -E copy_if_different
                "$<TARGET_PDB_FILE:${PROJECT_NAME}>"
                "${DLL_FOLDER}/$<TARGET_PDB_FILE_NAME:${PROJECT_NAME}>"
            VERBATIM)
    endif()
endif()
