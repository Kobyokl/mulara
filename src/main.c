#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "gbstructs.h"
#include "cpu_core.h"
#include "interrupt.h"

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

SharpState_t CPU_State = {0};
GBMemory_t GBMem = {0};
Cartridge_t cart = {0};

int main(int argc, char *argv[]){
    size_t ROMSIZE;
    int step_limit = 100000;
    int close_watcher = 0;
    FILE *RAW_ROM = fopen(argv[1], "rb"); // rb is for read binary btw..

    if(RAW_ROM == NULL){
        printf("Could not open ROM: %s\n", argv[1]);
        return 1;
    }



    fseek(RAW_ROM, 0, SEEK_END);
    ROMSIZE = ftell(RAW_ROM); // stores the file's size in bytes
    rewind(RAW_ROM);

    uint8_t *ROM = ProcessROM(RAW_ROM, ROMSIZE);

    Emulator_t main_emu = {
        .mem = GBMem,
        .CPU = CPU_State,

        .cartr = {
            .size = ROMSIZE,
            .ROM = ROM,
            .MBC_type = 0
        }
    };

    /*
        copy ROM into the memory that the bus actually reads from
    */
    size_t ROM_bytes = ROMSIZE;

    if(ROM_bytes > 0x8000){
        ROM_bytes = 0x8000;
    }

    memcpy(main_emu.mem.ROM, ROM, ROM_bytes); // put the memory read from the file into the memory mapped ROM memory ayee


    /*
        DMG CPU state after bootrom
    */
    main_emu.CPU.A = 0x01;
    main_emu.CPU.F = 0xB0;

    main_emu.CPU.BC = 0x0013;
    main_emu.CPU.DE = 0x00D8;
    main_emu.CPU.HL = 0x014D;

    main_emu.CPU.SP = 0xFFFE;
    main_emu.CPU.PC = 0x0100;

    main_emu.CPU.state = 'n';


    /*
        interrupt state
    */
    main_emu.mem.IME = 0;
    main_emu.mem.IME_next = 0;
    main_emu.mem.interrupt_enable = 0;
    main_emu.mem.CPU = &main_emu.CPU;

    main_emu.mem.IORegisters[0x0F] = 0;


    /*
        timer state
    */
    main_emu.mem.DIV = 0;
    main_emu.mem.sys_clock = 0;

    main_emu.mem.and_result = 0;
    main_emu.mem.prev_and_result = 0;

    main_emu.mem.TIMA_overflowing = 0;
    main_emu.mem.TIMA_reloading = 0;

    main_emu.mem.buffer_pointer = 0;


    int running = 1;

    while(running && (main_emu.CPU.state != 'l')){
        uint8_t IE = main_emu.mem.interrupt_enable & 0x1F;
        uint8_t IF = main_emu.mem.IORegisters[0x0F] & 0x1F;

        uint8_t pending = IE & IF; //pending an interrupt that needs to be serviced

        /*
            halted CPU behavior handlgin ggg
        */
        if(main_emu.CPU.state == 'h'){

            if(pending){
                main_emu.CPU.state = 'n';
                if(main_emu.mem.IME) process_interrupt(&main_emu.CPU, &main_emu.mem);

            }else{
                tick(&main_emu.mem); // just let a cycle pass and we gon service the interrupt latrrrrr
            }

            continue;
        }

        uint8_t enable_IME = main_emu.mem.IME_next;

        if(main_emu.mem.IME && pending){

            process_interrupt(&main_emu.CPU, &main_emu.mem);

        }else{


            uint16_t old_PC = main_emu.CPU.PC;

            handle_instruction(&main_emu.CPU, &main_emu.mem);


            if(old_PC == main_emu.CPU.PC){
                running = 0;
            }

            if(enable_IME && main_emu.mem.IME_next){ // properly handling IME setting
                main_emu.mem.IME = 1;
                main_emu.mem.IME_next = 0;
            }

        }

    }

    // printf("\n\n\n\nEMULATOR STOPPED\n");
    // printf("CPU state: %c\n", main_emu.CPU.state);
    // printf("PC: %04X\n", main_emu.CPU.PC);
    // printf("AF: %02X%02X\n", main_emu.CPU.A, main_emu.CPU.F);
    // printf("BC: %04X\n", main_emu.CPU.BC);
    // printf("DE: %04X\n", main_emu.CPU.DE);
    // printf("HL: %04X\n", main_emu.CPU.HL);
    // printf("SP: %04X\n", main_emu.CPU.SP);


    printf("\n__________________SERIAL OUTPUT__________________ \n \n %s \n \n ", main_emu.mem.serial_output);
}
