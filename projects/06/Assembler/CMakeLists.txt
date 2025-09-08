cmake_minimum_required(VERSION 3.10)

project(Assembler LANGUAGES CXX)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Aggiungi i file sorgente
set(SOURCES
    src/main.cpp
    src/Coder.cpp
    src/Parser.cpp
    src/Assembler.cpp
)

# Crea l'eseguibile
add_executable(Assembler ${SOURCES})

# Aggiungi directory include al target
target_include_directories(Assembler PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Aggiungi flag di compilazione al target
target_compile_options(Assembler PRIVATE
    -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -pedantic
)
