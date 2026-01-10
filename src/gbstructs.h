#include <stdint.h>

typedef struct SharpState{
    uint8_t A; //accumulator reguster
    uint8_t F; //flag register
    uint8_t int_enable;

    uint16_t BC;
    uint16_t DE;
    uint16_t HL;

    uint16_t SP; // stack pointer
    uint16_t PC; // program counter indexes to a byte in the array of uint8_t instructions
} SharpState_t;

typedef struct Cartridge{
    long  size;
    uint8_t *ROM;
    uint8_t MBC_type;
    //bank switching stuff... extra ram... blah blah blah
}Cartridge_t;

typedef struct GBMemory{
    uint8_t ROM[0x8000];
    uint8_t VRAM[0x2000];
    uint8_t WRAM[0x2000];
    uint8_t OAM[0xA0];
    uint8_t HRAM[0x7F];

    //etc
}GBMemory_t;
