#pragma once

#include <stdio.h>
#include <stdlib.h>

// a struct to keep track of cpu states
typedef struct SharpState {
    uint8_t A; // accumulator reguster
    uint8_t F; // flag register

    uint8_t IME; // interrupt master enable.,. this is also a flag
    uint8_t IME_next; // if this is 1 then IME is set to 1 in the following machine cycle

    uint16_t BC;
    uint16_t DE;
    uint16_t HL;

    uint16_t SP; // stack pointer
    uint16_t PC; // program counter indexes to a byte in the array of uint8_t instructions

    char state; // can be h, s, l, or n (halt, stopped, locked, or normal)


} SharpState_t;

// struct to store ROM info
typedef struct Cartridge {
    long size;
    uint8_t *ROM; // array of bytesss
    uint8_t MBC_type;
    //bank switching stuff... extra ram... blah blah blah
} Cartridge_t;

// the gb's memory thingy
typedef struct GBMemory {
    uint8_t ROM[0x8000];
    uint8_t VRAM[0x2000];
    uint8_t ExRAM[0x2000];
    uint8_t WRAM[0x2000];
    uint8_t OAM[0xA0];
    uint8_t HRAM[0x7F];


    uint8_t IORegisters[0xFFFF - 0xFF80];

    // in this array the i/o registers lie.... they can be accessed by indexing like
    // IORegisters[0x04] corresponds with the DIV register which is 0xFF04


    uint8_t interrupt_enable; // 0xFFFF in memory.
    uint8_t IME;
    //etc... prolly bank switching stuff goes here later

    unsigned int sys_clock; // this isn't in the gameboy's memory map but i'm putting it here to serve as a system clock for rn..
} GBMemory_t;


typedef struct Emulator { /// define ona these in the program entry point...
    GBMemory_t mem;
    SharpState_t CPU;
    Cartridge_t cartr;

} Emulator_t;

// put this somwhere else gng 
uint8_t* ProcessROM(FILE *ROM, size_t ROMSIZE);

// initialize global sharpstate and gbmem variables (implement later icl...)
extern const uint8_t gb_daa_table[256 * 8 * 2];

//global variables
extern SharpState_t CPU_State; // reflects state of cpu regs, state, and interrupts
extern GBMemory_t GBMem; // reflects state of gameboy memory
