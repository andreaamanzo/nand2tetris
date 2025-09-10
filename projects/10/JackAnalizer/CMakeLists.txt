cmake_minimum_required(VERSION 3.10)

project(JackAnalizer LANGUAGES CXX)

# Imposta lo standard C++
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Aggiungi i file sorgente
set(SOURCES
    src/main.cpp
    src/CompilationEngine.cpp
    src/JackAnalizer.cpp
    src/JackTokenizer.cpp
    src/KeyWords.cpp
)

# Crea l'eseguibile
add_executable(JackAnalizer ${SOURCES})

# Aggiungi directory include al target
target_include_directories(JackAnalizer PRIVATE ${CMAKE_SOURCE_DIR}/include)

# Aggiungi flag di compilazione al target
target_compile_options(JackAnalizer PRIVATE
    -Wall -Weffc++ -Wextra -Wconversion -Wsign-conversion -pedantic
)
