# Nand2Tetris — Solutions & Implementations (C++17)

This repository collects my **solutions** and **implementations** for the **[Nand2Tetris](https://www.nand2tetris.org)** project/course. It includes HDL/hardware work, assembly programs, and **C++** tools for the Hack platform.

## Tools implementation (index)
- `06/Assembler/` — C++17 **Assembler** (ASM → HACK)
- `07/VMtranslator/` — C++17 **VM translator** (VM → ASM)

## Build (tools)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```
### Requirements
- CMake **3.15+**
- A **C++17** compiler 

## Usage
### Assembler (ASM → HACK)
```bash
# from: 06/Assembler/build

./Assembler path/to/Program.asm
# Output: path/to/Program.hack
```

### VM translator (VM → ASM)
```bash
# from: 07/VMtranslator/build

# Single file
./VMtranslator path/to/File.vm
# Output: path/to/File.asm

# Directory
./VMtranslator path/to/Dir/
# Output: path/to/Dir/Dir.asm
```