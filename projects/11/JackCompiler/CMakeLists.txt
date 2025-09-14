cmake_minimum_required(VERSION 3.15)

project(JackCompiler LANGUAGES CXX)

# Standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Colleziona i sorgenti automaticamente
file(GLOB_RECURSE SOURCES
    src/main.cpp
    src/**/*.cpp
)

# Crea l'eseguibile
add_executable(JackCompiler ${SOURCES})

# Directory include pubblica
target_include_directories(JackCompiler
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Opzioni di compilazione
target_compile_options(JackCompiler PRIVATE
    -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion
)
