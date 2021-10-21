# =======================================================================
#   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
# =======================================================================
#   Remember to put on SAFETY GOGGLES before looking at this file. You
#   are most certainly not expected to read or understand any of it.
# =======================================================================
#
# This CMake file is responsible for compiling dependency libraries and
# setting up suitable compiler flags for various platforms. You do not
# need to read or change anything in this file; see CMakeLists.txt in
# the main Nori folder instead.

include(CheckCXXCompilerFlag)

# Set a default build configuration (Release)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    message(STATUS "Setting build type to 'Release' as none was specified.")
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
        "MinSizeRel" "RelWithDebInfo")
endif()
string(TOUPPER "${CMAKE_BUILD_TYPE}" U_CMAKE_BUILD_TYPE)

# Enable folders for projects in Visual Studio
if (CMAKE_GENERATOR MATCHES "Visual Studio")
    set_property(GLOBAL PROPERTY USE_FOLDERS ON)
endif()

if (APPLE)
  set(CMAKE_MACOSX_RPATH ON)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGL_SILENCE_DEPRECATION=1")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_CPP14_FLAG)
    CHECK_CXX_COMPILER_FLAG("-std=c++11" HAS_CPP11_FLAG)

    if (HAS_CPP14_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
    elseif (HAS_CPP11_FLAG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    else()
        message(FATAL_ERROR "Unsupported compiler -- Mitsuba requires C++11 support!")
    endif()

    # Enable link time optimization and set the default symbol
    # visibility to hidden (very important to obtain small binaries)
    if (NOT ${U_CMAKE_BUILD_TYPE} MATCHES DEBUG)
        # Default symbol visibility
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    endif()
endif()

# Sanitize build environment for static build with C++11
if (MSVC)
    # Disable annoying secure CRT warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_CRT_SECURE_NO_WARNINGS")

    # Parallel build on MSVC (all targets)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

    if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")

        # Disable Eigen vectorization for Windows 32 bit builds (issues with unaligned access segfaults)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEIGEN_DONT_ALIGN")
    endif()

    # Static build
    set(CompilerFlags
        CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
        CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
    foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
    endforeach()
endif()

if (WIN32)
    # Build zlib (only on Windows)
    set(ZLIB_BUILD_STATIC_LIBS ON CACHE BOOL " " FORCE)
    set(ZLIB_BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
    add_subdirectory(zlib)

    set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE PATH " " FORCE)
    set(ZLIB_LIBRARY "${CMAKE_CURRENT_BINARY_DIR}/zlib/$<CONFIGURATION>/zlibstatic.lib" CACHE FILEPATH " " FORCE)

    set_property(TARGET zlibstatic PROPERTY FOLDER "dependencies")
    include_directories(${ZLIB_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/zlib")
endif()

# Build NanoGUI
set(NANOGUI_BUILD_EXAMPLE OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_SHARED  OFF CACHE BOOL " " FORCE)
set(NANOGUI_BUILD_PYTHON  OFF CACHE BOOL " " FORCE)
add_subdirectory(nanogui)
set_property(TARGET glfw glfw_objects nanogui PROPERTY FOLDER "dependencies")

# Build the pugixml parser
add_library(pugixml STATIC pugixml/src/pugixml.cpp)
set_property(TARGET pugixml PROPERTY
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pugixml")
set_property(TARGET pugixml PROPERTY FOLDER "dependencies")

# Build tinyobjloader
set(TINYOBJLOADER_BUILD_TEST_LOADER OFF CACHE BOOL " " FORCE)
set(TINYOBJLOADER_BUILD_OBJ_STICHER  OFF CACHE BOOL " " FORCE)
add_subdirectory(tinyobjloader)

# Build lodepng
add_subdirectory(lodepng)

set(TFM_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyformat)
set(GLFW_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glfw/include)
set(GLEW_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glew/include)
set(NANOVG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/nanovg/src)
set(NANOGUI_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/include)
set(EIGEN_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/eigen)
set(FILESYSTEM_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem)
set(PUGIXML_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/pugixml/src)
set(TINYOBJLOADER_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyobjloader)
set(TINYEXR_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/tinyexr)
set(LODEPNG_INCLUDE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/lodepng)

if(WIN32)
    set(GLAD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/nanogui/ext/glad/include")
endif()

# Compile remainder of the codebase with compiler warnings turned on
if(MSVC)
    if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
        string(REGEX REPLACE "/W[0-4]" "/W4" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    endif()
elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter")
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-gnu-anonymous-struct -Wno-c99-extensions -Wno-nested-anon-types -Wno-deprecated-register")
    endif()
endif()

set(CompilerFlags
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS
    CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_C_FLAGS_RELWITHDEBINFO COMPILE_DEFINITIONS U_CMAKE_BUILD_TYPE
    CMAKE_MACOSX_RPATH
    TFM_INCLUDE_DIR GLFW_INCLUDE_DIR GLEW_INCLUDE_DIR
    NANOVG_INCLUDE_DIR NANOGUI_INCLUDE_DIR EIGEN_INCLUDE_DIR
    FILESYSTEM_INCLUDE_DIR PUGIXML_INCLUDE_DIR TINYOBJLOADER_INCLUDE_DIR
    LODEPNG_INCLUDE_DIR TINYEXR_INCLUDE_DIR GLAD_INCLUDE_DIR
    NANOGUI_EXTRA_LIBS
    )
foreach(CompilerFlag ${CompilerFlags})
    set(${CompilerFlag} "${${CompilerFlag}}" PARENT_SCOPE)
endforeach()
