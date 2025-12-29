#include <stdio.h>
#include <stdint.h>

typedef struct{
    uint8_t A; //accumulator reguster
    uint8_t B; // gen reg
    uint8_t C; // gen reg
    uint8_t D; // gen reg
    uint8_t E; // gen reg
    uint8_t H; // gen reg
    uint8_t L; // gen reg
    uint8_t F; //flag register

    uint16_t *SP; // stack pointer
    uint16_t *PC; // program register

} SharpState_t;

int SM83_disassembler( SharpState_t *CPU_State){
    /*
        Given a certain opcode evaluate the opcode and update the program counter accordingly

        Returns the number of machine cycles the instruction used
    */
    int machine_cycles;

    uint16_t *OpCode = CPU_State->PC;
    // OpCode and CPU_State-PC reference the same block of memory
    switch(*OpCode) {
        case 0xC3:
        CPU_State->PC = CPU_State->
    }


    return machine_cycles;

}

int main(void){
    SharpState_t sharpState = { //initializes the state of the cpu
        .A = 0,
        .B = 0,
        .C = 0,
        .D = 0,
        .E = 0,
        .H = 0,
        .L = 0,

        // written as binary number to better see each bit (cuz they r important)
        .F = 0b00000000,

        .SP = 0x0000,
        .PC = 0x0000,
    };


    return 0;

}
