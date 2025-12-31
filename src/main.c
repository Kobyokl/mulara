#include <stdio.h>
#include <stdint.h>

typedef struct{
    uint8_t A; //accumulator reguster
    uint8_t F; //flag register

    uint16_t BC;
    uint16_t DE;
    uint16_t HL;

    uint16_t SP; // stack pointer
    uint16_t PC; // program register

} SharpState_t;

int SM83_disassembler(SharpState_t *CPU_State){
    /*
        Given a certain opcode evaluate the opcode and update the program counter accordingly

        Returns the number of machine cycles the instruction used
    */
    // OpCode and CPU_State-PC reference the same block of memory

    switch(CPU_State->PC) {

        case 0x01:
        //
        case 0xC3:
        // Opcode + LSB(nn) + MSB(nn) [together form a 16 bit instruction address]
            printf("Jump to memory address : ", CPU_State->PC + 0); //should be the next word in the .gb file
            return 4;
    }

}

int main(int argc, char *argv[]){
    SharpState_t sharpState = { //initializes the state of the cpu
        .A = 0x01,

        .BC = 0x0013,
        .DE = 0x00d8,
        .HL = 0x014D,

        // written as binary number to better see each bit (cuz they r important)
        .F = 0b00000000,

        .PC = 0x0100,
        .SP = 0xfffe,
    };

    const FILE *ROM = fopen(argv[1], "rb");
    if(ROM == NULL){
        printf("\nROM not loaded\n");
        return -1;
    }

    int byte;
    size_t count = 0; //size_t is basically used as a unsigned int... better than limiting it's size cuz. we dont know how big a rom is gonna b because of mapping

    // Read file byte-by-byte
    while((byte = fgetc(ROM)) != EOF){
        printf("%02X", (unsigned char) byte);
        count++;

        if(count % 16  == 0){
            printf("\n");
        }
    }
    fclose(ROM);
    printf("\n Bytes Read: %zu \n", count);
    return 0;

}
