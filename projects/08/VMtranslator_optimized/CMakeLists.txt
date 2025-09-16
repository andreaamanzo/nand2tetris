cmake_minimum_required(VERSION 3.10)

project(VMtranslator LANGUAGES CXX)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Aggiungi i file sorgente
set(SOURCES
    src/main.cpp
    src/CodeWriter.cpp
    src/Parser.cpp
    src/Translator.cpp
)

# Crea l'eseguibile
add_executable(VMtranslator ${SOURCES})

# Aggiungi directory include al target
target_include_directories(VMtranslator PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Aggiungi flag di compilazione al target
target_compile_options(VMtranslator PRIVATE
    -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -pedantic
)
