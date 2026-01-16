#include "bus.h"
uint8_t block_finder(uint8_t opcode) { // turn this into a macro later
    /*
        returns a number 0 - 3 that will categorize an instruction from blocks 0 - 3
    */
    return (opcode & 0b11000000) >> 6;
}
void set_16bit_reg(SharpState_t *CPU, uint16_t value, uint8_t identifier){
    switch(identifier){
        case(0): CPU->BC = value;
        case(1): CPU->DE = value;
        case(2): CPU->HL = value;
        case(3): CPU->SP = value;
    }
}

void set_8bit_reg(SharpState_t *CPU, uint16_t value, uint8_t identifier){
    switch(identifier){
        case(0): CPU->BC = (CPU->BC & 0x00FF) | value; // b
        case(1): CPU->DE = (CPU->DE & 0x00FF) | value; // d
        case(2): CPU->HL = (CPU->HL & 0x00FF) | value; // h
        // handle AF
    }
}


void better_instruction_handler(SharpState_t *CPU_State, GBMemory_t *mem){
    uint8_t opcode = fetch_byte(mem, CPU_State);
    uint8_t block = block_finder(opcode);

    switch(block){
        case(0): // block 0
        {
            if(opcode == 0){ //nop
                break;
            }else if(opcode == 16){ //stop
                CPU_State->state = 's';
                break;
            }

            switch(opcode & 0x0F){
                case(1): // ld r16, imm16
                {
                    uint8_t value = fetch_byte(mem, CPU_State);
                    uint8_t tbi = (opcode & 0b00111000) >> 4;

                }
            }


        }
    }
}
