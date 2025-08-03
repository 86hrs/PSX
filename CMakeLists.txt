cmake_minimum_required(VERSION 3.10)

project(psx VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/obj)

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
    src/r3000d.h
    src/r3000d.c
)

set_target_properties(${PROJECT_NAME} PROPERTIES INTERPROCEDURAL_OPTIMIZATION TRUE)
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -march=native
    -flto
    -fno-rtti             # If you don’t use dynamic_cast or typeid
)

