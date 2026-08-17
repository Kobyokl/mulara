#include <stdio.h>
#include <stdint.h>

#include "gbstructs.h"
#include "cpu_core.h"

//prolly finna delete or move this file somewhere else idk..


uint8_t* ProcessROM(FILE *ROM, size_t ROMSIZE){
    /*
        processes a ROM file and turns it into an array of uint8's
    */
    if(ROM == NULL){
        printf("\nROM not loaded\n");
        return NULL;
    }

    // Read file byte by byte
    uint8_t *ROMBuffer = malloc(ROMSIZE); // array the size of ROM in heap

    fread(ROMBuffer, 1, ROMSIZE, ROM); // put the ROM in the array byte by byte
    fclose(ROM);

    return ROMBuffer;
}


// maybe move these somewhere else
extern SharpState_t sharpstate;
extern GBMemory_t GBMem;
extern Cartridge_t cart;

int main(int argc, char *argv[]){
    size_t ROMSIZE;
    FILE *RAW_ROM = fopen(argv[1], "rb"); // rb is for read binary btw..

    fseek(RAW_ROM, 0, SEEK_END);
    ROMSIZE = ftell(RAW_ROM); // stores the file's size in bytes
    rewind(RAW_ROM);

    uint8_t *ROM = ProcessROM(RAW_ROM, ROMSIZE);

    Emulator_t main_emu = {
        .mem = GBMem,
        .CPU = sharpstate,
        .cartr = cart
    };

    for(int i = 0; i < ROMSIZE; i++){
        printf("Current byte in ROM: %0X", ROM[i]);
    }

    int running = 1;

    while(running && (main_emu.CPU.state != 'l')){

        if(main_emu.mem.IME){
            uint8_t IE = main_emu.mem.interrupt_enable & 0x1F;
            uint8_t IF = main_emu.mem.IORegisters[0x0F] & 0x1F;

            if(IE & IF){ // check if there are any interrupts that need to be serviced before processing next instruction
                process_interrupt(&main_emu.CPU, &main_emu.mem);
            }else{
                instruction_step(&main_emu.CPU, &main_emu.mem);
            }
        }else{
            instruction_step(&main_emu.CPU, &main_emu.mem);
        }
    }

}
