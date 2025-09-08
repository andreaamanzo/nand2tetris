// This file is part of www.nand2tetris.org
// and the book "The Elements of Computing Systems"
// by Nisan and Schocken, MIT Press.
// File name: projects/4/Fill.asm

// Runs an infinite loop that listens to the keyboard input. 
// When a key is pressed (any key), the program blackens the screen,
// i.e. writes "black" in every pixel. When no key is pressed, 
// the screen should be cleared.


@SCREEN
D=A
// (256x512)/16 = 8192
@8192
D=D+A
@SCREEN_END
M=D

(INIT)
    @SCREEN
    D=A
    @pixel
    M=D
    
    @KBD
    D=M
    @WHITE
    D;JEQ

    @color
    //sets all the bits to 1
    M=-1

    @DRAW
    0;JMP

(WHITE)
    @color
    M=0

(DRAW)
    //stop if pixel == screen_end
    @pixel
    D=M
    @SCREEN_END
    D=D-M
    @INIT
    D;JGE

    @color
    D=M
    @pixel
    A=M
    M=D    

    @pixel
    M=M+1

    @DRAW
    0;JMP
