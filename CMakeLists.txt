cmake_minimum_required(VERSION 3.16)

project(
    jack-compiler
    VERSION 0.0.1
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Create the main target
add_executable(${PROJECT_NAME})

add_subdirectory(src)
