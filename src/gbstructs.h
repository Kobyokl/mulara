#pragma once

#include <stdint.h>

// a struct to keep track of cpu states
typedef struct SharpState{
    uint8_t A; //accumulator reguster
    uint8_t F; //flag register
    uint8_t CB_Prefix; // keep track of whether

    uint16_t BC;
    uint16_t DE;
    uint16_t HL;

    uint16_t SP; // stack pointer
    uint16_t PC; // program counter indexes to a byte in the array of uint8_t instructions

    char state; // can be h, s, or n (halt, stopped, or normal)

} SharpState_t;

// struct to store ROM info
typedef struct Cartridge{
    long size;
    uint8_t *ROM;
    uint8_t MBC_type;
    //bank switching stuff... extra ram... blah blah blah
} Cartridge_t;

// the gb's memory thingy
typedef struct GBMemory{
    uint8_t ROM[0x8000];
    uint8_t VRAM[0x2000];
    uint8_t WRAM[0x2000];
    uint8_t OAM[0xA0];
    uint8_t HRAM[0x7F];
    uint8_t IORegisters[0xFFFF - 0xFF80];
    uint8_t InterruptEnable;
    //etc
} GBMemory_t;

// put this somwhere else gng :sob:
uint8_t* ProcessROM(FILE *ROM, size_t ROMSIZE);

// initialize global sharpstate and gbmem variables (implement later icl...)
extern SharpState_t sharpstate;
extern GBMemory_t GBMem;
