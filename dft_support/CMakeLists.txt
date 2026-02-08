cmake_minimum_required(VERSION 3.10)
project(dsplib C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Define _USE_MATH_DEFINES for M_PI on Windows
if(WIN32)
    add_definitions(-D_USE_MATH_DEFINES)
endif()

# Option to enable/disable SndFile support
option(ENABLE_SNDFILE "Enable libsndfile support" ON)

# Configuration for finding libsndfile
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/usr/local/Cellar/libsndfile/1.0.30_1")
endif()

if(ENABLE_SNDFILE)
    # Try to find SndFile using the standard package first
    find_package(SndFile QUIET)

    if(NOT SndFile_FOUND)
        # Fallback: Manual search in common locations or user-provided path
        # Check both SNDFILE_ROOT and possible subfolders
        find_path(SNDFILE_INCLUDE_DIR NAMES sndfile.h PATHS ${SNDFILE_ROOT}/include ${SNDFILE_ROOT})
        find_library(SNDFILE_LIBRARY NAMES sndfile sndfile-1 libsndfile PATHS ${SNDFILE_ROOT}/lib ${SNDFILE_ROOT})
        
        if(SNDFILE_INCLUDE_DIR AND SNDFILE_LIBRARY)
            set(SndFile_FOUND TRUE)
            add_library(SndFile::sndfile UNKNOWN IMPORTED)
            set_target_properties(SndFile::sndfile PROPERTIES
                IMPORTED_LOCATION "${SNDFILE_LIBRARY}"
                INTERFACE_INCLUDE_DIRECTORIES "${SNDFILE_INCLUDE_DIR}"
            )
        endif()
    endif()

    if(SndFile_FOUND)
        message(STATUS "Found SndFile!")
        add_definitions(-DHAVE_SNDFILE)
    else()
        message(WARNING "SndFile not found. Building without sound file support. 
        TIP: Use the CORRECT path to the extracted folder: 
        cmake -B build -DSNDFILE_ROOT=\"C:/git/libsndfile\"")
    endif()
endif()

# Common include directory (Project Root)
set(PROJ_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${PROJ_ROOT})

# Set directories for better organization (Relative to PROJ_ROOT)
set(DFT_DIR ${PROJ_ROOT}/dft)
set(UTIL_DIR ${PROJ_ROOT}/util)
set(CPLX_MATH_DIR ${PROJ_ROOT}/cplx_math)
set(DFT_SUPPORT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

# DFT Frontend executable
add_executable(dft_frontend
    ${DFT_SUPPORT_DIR}/frontend/dft_frontend.c
    ${DFT_DIR}/src/dft.c
    ${DFT_DIR}/src/file.c
    ${UTIL_DIR}/src/util.c
    ${CPLX_MATH_DIR}/src/cplx_math.c
)

# Link libraries
if(SndFile_FOUND)
    target_link_libraries(dft_frontend PRIVATE SndFile::sndfile)
    
    # On Windows, copy the DLL to the output directory
    if(WIN32)
        # Try to find the DLL near the library or in bin
        get_filename_component(SNDFILE_LIB_DIR "${SNDFILE_LIBRARY}" DIRECTORY)
        find_file(SNDFILE_DLL NAMES sndfile.dll PATHS "${SNDFILE_LIB_DIR}/../bin" "${SNDFILE_LIB_DIR}")
        
        if(SNDFILE_DLL)
            message(STATUS "Found SndFile DLL: ${SNDFILE_DLL}")
            add_custom_command(TARGET dft_frontend POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${SNDFILE_DLL}"
                "$<TARGET_FILE_DIR:dft_frontend>")
        endif()
    endif()
endif()

# Math library is usually needed on Linux/Unix
if(UNIX AND NOT APPLE)
    target_link_libraries(dft_frontend PRIVATE m)
endif()

# Installation/Output settings
set_target_properties(dft_frontend PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
