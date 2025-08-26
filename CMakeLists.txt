cmake_minimum_required(VERSION 3.10)

project(psx VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)
 
# Find OpenGL
set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

# GLFW Configuration - Only build if not already built
if(NOT TARGET glfw)
    # Download and build GLFW only once
    include(FetchContent)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
        GIT_SHALLOW TRUE  # Only fetch the latest commit for this tag
    )
    
    # Set these BEFORE FetchContent_MakeAvailable
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    
    FetchContent_MakeAvailable(glfw)
    
    # Mark GLFW as excluded from 'all' target
    set_target_properties(glfw PROPERTIES EXCLUDE_FROM_ALL TRUE)
endif()

# Find Freetype
find_package(Freetype REQUIRED)

# ImGui setup
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/imgui)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/backends/imgui_impl_glfw.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)



# Add executable (replace with your actual source files)
add_executable(${PROJECT_NAME}
    src/main.cc
    src/cpu.cc
    src/cpu.h
    src/bios.h
    src/bios.cc
    src/interconnect.h
    src/interconnect.cc
    src/map.h
    src/ram.h
    src/ram.cc
    src/dma.h
    src/dma.cc
    src/channel.h
    src/channel.cc
    src/gpu.h
    src/gpu.cc
    src/r3000d.h
    src/r3000d.c
    src/commandbuffer.h
    src/commandbuffer.cc
    src/glad.h
    src/glad.c
    src/khrplatform.h
    src/shader.h
    src/shader.cc
    ${IMGUI_SOURCES}
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    src
    /opt/homebrew/Cellar/glm/1.0.1/include/
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${FREETYPE_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    ${OPENGL_LIBRARIES}
    glfw
    ${FREETYPE_LIBRARIES}
)

# set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
# target_compile_options(${PROJECT_NAME} PRIVATE
#     -O3
#     -march=native
#     -flto
#     -fno-rtti             # If you don’t use dynamic_cast or typeid
# )

