cmake_minimum_required(VERSION 3.5)
project(turingViz)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the exact path to your FLTK installation
set(FLTK_DIR "/opt/homebrew/opt/fltk")

# Include directories for FLTK and your project sources
find_path(FLTK_INCLUDE_DIR FL/Fl.H PATHS /opt/homebrew/include)
include_directories(${FLTK_INCLUDE_DIR})
include_directories(/Users/benmeyers/Desktop/turingViz/src)
include_directories(/Users/benmeyers/Desktop/turingViz/src/graphics)
include_directories(/Users/benmeyers/Desktop/turingViz/src/TuringMachine)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} “${CMAKE_SOURCE_DIR}/cmake/Modules”)

# Find and link with the required FLTK libraries
file(GLOB FLTK_LIBS "${FLTK_DIR}/lib/libfltk*.dylib")
message(STATUS "Found FLTK libraries: ${FLTK_LIBS}")

find_package(nlohmann_json REQUIRED)

# Add your main executable
add_executable(turingViz main.cpp)
# Add this after add_executable in your CMakeLists.txt
file(GLOB GRAPHICS_SRC "/Users/benmeyers/Desktop/turingViz/src/*.cpp" "/Users/benmeyers/Desktop/turingViz/src/graphics/*.cpp" )
target_sources(turingViz PRIVATE ${GRAPHICS_SRC})

# Link your static/dynamic libraries (adjust paths as needed)
target_link_libraries(turingViz
    # /Users/benmeyers/Desktop/turingViz/build/libgraphics.a
    ${FLTK_LIBS}
    nlohmann_json::nlohmann_json
)

# For macOS, link system frameworks
if(APPLE)
    find_library(COCOA_LIBRARY Cocoa REQUIRED)
    find_library(CARBON_LIBRARY Carbon REQUIRED)
    find_library(IOKIT_LIBRARY IOKit REQUIRED)
    find_library(COREVIDEO_LIBRARY CoreVideo REQUIRED)
    find_library(OPENGL_LIBRARY OpenGL REQUIRED)
    target_link_libraries(turingViz
        ${COCOA_LIBRARY}
        ${CARBON_LIBRARY}
        ${IOKIT_LIBRARY}
        ${COREVIDEO_LIBRARY}
        ${OPENGL_LIBRARY}
    )
endif()

message(STATUS "FLTK_DIR: ${FLTK_DIR}")
message(STATUS "FLTK include dir: ${FLTK_DIR}/include")
message(STATUS "C++ compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")