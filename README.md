# Nand2Tetris — Solutions & Implementations (C++20)

This repository contains my **solutions** and **implementations** for the [Nand2Tetris](https://www.nand2tetris.org) course.  
It covers both **hardware-level designs** (HDL) and **software tools** (Assembler, VM Translator, Jack Compiler components), all implemented in **C++20**.  
Additionally, it includes projects written in **Jack**, the high-level language introduced in the course.

---

## 📂 Project Index

### 01–05: Hardware (HDL + ASM)
- Implemented in the supplied Hardware Description Language.  
- Built components bottom-up, starting from logic gates (NAND) and reaching:
  - Arithmetic units (Adders, ALU).  
  - Sequential components (Registers, RAM).  
  - Full **Hack CPU** and **Hack Computer**.  
- Wrote two Hack Assembly programs (`.asm`) to test the platform.  

### 06: Assembler (ASM → HACK)
- A full Hack **Assembler** in C++20.  
- Translates `.asm` files into `.hack` binary code.  

### 07–08: VM Translator (VM → ASM)
- A **VM Translator** in C++20, implementing the Nand2Tetris Virtual Machine.  
- Input: `.vm` files (single or multiple).  
- Output: `.asm` (Hack Assembly).  

### 09: Jack Programs (Jack)
- High-level projects written in the **Jack programming language**.   
- Implemented my own project:  
  - **Snake Game** (`Main.jack` + support classes).  
  - Features:
    - Real-time input handling.  
    - Snake movement and growth logic.  
    - Collision detection with walls and self.  
    - Score tracking and simple game loop.  
  - Serves as a complete test case for the VM Translator and Hack platform. 
- **How to test Snake**:  
  You can compile and run the game using the official Nand2Tetris web IDE:  
  [https://nand2tetris.github.io/web-ide/compiler](https://nand2tetris.github.io/web-ide/compiler)  
  1. Upload the entire `snake/src` folder.  
  2. Compile the project.  
  3. Click Run to open the generated `.vm` files in the **VM Emulator**.  
  4. Before running:  
     - Set the execution speed to **maximum**.  
     - Enable the **keyboard input** option.  
  5. Run the program and play Snake.

### 10: Jack Analyzer (Jack → XML)
- First stage of the **Jack Compiler** in C++20.  
- Components:
  - **Tokenizer** — converts source into a stream of tokens.
  - **Parser** — builds a structured representation of Jack programs.  
- Supports both **single `.jack` files** and **entire directories** (recursively).  
- Output files are generated inside an automatically created directory named `out_analizer`.
- **Output files**:
  - For each `Xxx.jack` source file:
    - `XxxT.xml`: contains the raw tokens produced by the **Tokenizer**.  
    - `Xxx.xml`: contains the hierarchical syntax structure produced by the **Parser**.  
  - Both are in XML format, suitable for comparison with the supplied reference files (`XxxT.xml` and `Xxx.xml`). 
- Purpose: validation of parsing correctness before full compilation.  

### 11: Jack Compiler (Jack → VM)
- Second stage of the **Jack Compiler** in C++20.  
- Generates **VM code** directly from Jack programs, implementing all compilation rules of the Jack language.  
- Components:
  - **Tokenizer** — manages Jack tokens.  
  - **Compilation Engine** — translates syntax into VM commands.  
  - **VM Writer** — produces VM code according to the VM specification.  
  - **Symbol Table** — manages symbol definitions and scopes.  
- **Input**: single `.jack` file or an entire directory of Jack sources.  
- **Output**: `.vm` file(s), one for each input file, generated inside an automatically created directory named `out`  
---

## ⚙️ Build Instructions

### Requirements
- **CMake 3.15+**  
- **C++20 compiler**

### Build
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

---

## 💻 Usage

### Assembler (ASM → HACK)
```bash
# from: 06/Assembler/build

./Assembler path/to/Program.asm
# Output: path/to/Program.hack
```

### VM Translator (VM → ASM)
```bash
# from: 08/VMtranslator/build

# Single file
./VMtranslator path/to/File.vm
# Output: path/to/File.asm

# Directory (recursively)
./VMtranslator path/to/Dir/
# Output: path/to/Dir/Dir.asm
```

### Jack Analyzer (Jack → XML)
```bash
# from: 10/JackAnalyzer/build

# Single file
./JackAnalyzer path/to/File.jack
# Output: path/to/out_analizer/File(T).xml

# Directory (recursively)
./JackAnalyzer path/to/ProjectDir/
# Output: path/to/ProjectDir/out_analizer/*.xml
```

### Jack Compiler (Jack → VM)
```bash
# from: 11/JackCompiler/build

# Single file
./JackCompiler path/to/File.jack
# Output: path/to/out/File.vm

# Entire directory
./JackCompiler path/to/ProjectDir/
# Output: path/to/ProjectDir/out/*.vm
```

---

## 📖 Progress Status

- ✅ Hardware & ASM programs (Projects 1–5)  
- ✅ Assembler (Project 6)  
- ✅ VM Translator (Projects 7–8)  
- ✅ Jack Programs (Project 9)  
- ✅ Jack Analyzer (Project 10)  
- ✅ Jack Compiler (Projects 11)  
- 🔜 Jack OS Implementations (Project 12)  