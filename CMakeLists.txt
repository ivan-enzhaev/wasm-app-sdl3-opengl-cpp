cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(wasm_app_sdl3_opengl_cpp CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -----------------------------------------------------------------------------
# Paths (points directly to the folder containing SDL3Config.cmake)
# -----------------------------------------------------------------------------
set(SDL3_DIR "C:/libs/sdl3-3.4.18-wasm/lib/cmake/SDL3")

# -----------------------------------------------------------------------------
# Locate Official SDL3 CMake Package
# -----------------------------------------------------------------------------
find_package(SDL3 REQUIRED CONFIG PATHS "${SDL3_DIR}" NO_CMAKE_FIND_ROOT_PATH)

# -----------------------------------------------------------------------------
# WebAssembly Executable Target
# -----------------------------------------------------------------------------
add_executable(app
    src/main.cpp
)

set_target_properties(app PROPERTIES
    SUFFIX ".html"
)

# Automatically generate index.html by copying app.html
# This allows http://localhost:8000/ to open directly without a directory listing
add_custom_command(TARGET app POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_BINARY_DIR}/app.html"
        "${CMAKE_CURRENT_BINARY_DIR}/index.html"
    COMMENT "Generating index.html from app.html for default root server loading"
)

# -----------------------------------------------------------------------------
# Link SDL3 Static Library
# -----------------------------------------------------------------------------
target_link_libraries(app PRIVATE
    SDL3::SDL3-static
)

# -----------------------------------------------------------------------------
# Emscripten Link Flags (WebGL 2.0 & Dynamic Memory)
# -----------------------------------------------------------------------------
target_link_options(app PRIVATE
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=2
    -sALLOW_MEMORY_GROWTH=1
)