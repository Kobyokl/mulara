#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// a struct to keep track of cpu states
typedef struct SharpState {
    int cgb_mode;

    uint8_t A; // accumulator reguster
    uint8_t F; // flag register

    uint16_t BC;
    uint16_t DE;
    uint16_t HL;

    uint16_t SP; // stack pointer
    uint16_t PC; // program counter indexes to a byte in the array of uint8_t instructions



    char double_speed; // are we in double speed

    char state; // can be h, s, l, w, or n (halt, stopped, locked, weird [for speed swiching], or normal)
    unsigned int wm_cycle; // records the m-cycle that the gameboy had entered the weird state that occurs on a speed switch... this weird state is switch out of after 2050 M-cycles


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
    uint8_t VRAM[2][0x2000];
    uint8_t ExRAM[0x2000];
    uint8_t WRAM0[0x1000];
    uint8_t WRAMX[8][0x1000];
    uint8_t OAM[0xA0];
    uint8_t HRAM[0x7F];


    uint8_t IORegisters[0x80];
    uint16_t DIV; // we dont actually use IORegisters[0x04] bc that's actuallly a 16 bit value

    // in this array the i/o registers lie.... they can be accessed by indexing like
    // IORegisters[0x04] corresponds with the DIV register which is 0xFF04


    uint8_t interrupt_enable; // 0xFFFF in memory.
    uint8_t IME;
    uint8_t IME_next;
    //etc... prolly bank switching stuff goes here later

    VDMA_t vdma;

    unsigned int sys_clock; // this isn't in the gameboy's memory map but i'm putting it here to serve as a system clock for rn..
    unsigned int and_result; // for the sake of handling obscure timer operation
    unsigned int prev_and_result;
    unsigned int TIMA_overflowing; // if TIMA overflows then update its value one cycle later so we need this variabel to keep track of that state
    unsigned int TIMA_reloading;

    //debugging stuff
    SharpState_t *CPU; // cuz im lazy and a mid dev (will do better in future)
    char serial_output[200];
    int buffer_pointer;
} GBMemory_t;


typedef struct Emulator { /// define ona these in the program entry point...
    GBMemory_t mem;
    SharpState_t CPU;
    Cartridge_t cartr;
} Emulator_t;

// struct for initializing VDMA shi
typedef struct VDMA {
    unsigned int bytes_remaining; // how many bytes we gotta copy ?
    char hblank_mode; //  0 for GDMA and 1 for the other one
    char active; // r we doign the  dma or naw
    uint16_t source; // where in memory are we going to start to copy to VRAM /
    uint16_t dest; // where r we gonna put it
} VDMA_t;

// initialize global sharpstate and gbmem variables (implement later icl...)
extern const uint8_t gb_daa_table[256 * 8 * 2];

//global variables
extern SharpState_t CPU_State; // reflects state of cpu regs, state, and interrupts
extern GBMemory_t GBMem; // reflects state of gameboy memory
extern Cartridge_t cart;
