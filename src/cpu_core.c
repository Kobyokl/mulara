#include "cpu_core.h"

uint8_t read_byte(GBMemory_t *mem, uint16_t addr){
    if(addr < 0x8000){
        return mem->ROM[addr];
    }else if(addr < 0xA000){
        //VRAM
        return mem->VRAM[addr - 0x8000];

    }else if(addr < 0xC000){
        //ExRAM (on cartridge)

    }else if(addr < 0xE000){
        //WRAM
        return mem->WRAM[addr - 0xC000];

    }else if(addr < 0xFE00){
        //Mirror of WRAM
        return mem->WRAM[addr - 0xE000];
    }else if(addr < 0xFEA0){
        //OAM
    }else if(addr < 0xFF00){
        //Forbidden memory
    }else if(addr < 0xFF80){

    }else if(addr < 0xFFFF){
        //HRAM
    }else{
        //Interrupt Enable Register
    }
    //etc
}

void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value){
    if(addr < 0x8000){
        //ROM
    }else if(addr < 0xA000){
        //VRAM
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
    }else if(addr < 0xE000){
        //WRAM
    }else if(addr < 0xFE00){
        //Mirror of WRAM
    }else if(addr < 0xFEA0){
        //OAM
    }else if(addr < 0xFF00){
        //Forbidden memory
    }else if(addr < 0xFF80){
        switch(addr){
            case 0xFF46:
                // start_dma(value);
                break;
            case 0xFF04:
                // div = 0; // writing resets DIV
                break;
            default:
                // io[addr - 0xFF00] = value;

        }
    }else if(addr < 0xFFFF){
        //HRAM
    }else{
        //Interrupt Enable Register
    }
}

uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State){
    /*
        returns opcode and increments program counter
    */
    CPU_State->PC++; 
    return read_byte(mem, CPU_State->PC);
}

void sm83_instructionHandler(GBMemory_t *mem, SharpState_t *CPU_State){

       uint8_t opcode = fetch_byte(mem, CPU_State);

        switch(opcode){
            case(0x01):

            // case(0xC3):
            //         uint8_t LSB = ROM[CPU_State->PC + 1];
            //         uint8_t MSB = ROM[CPU_State->PC + 2];
            //         CPU_State->PC = (uint16_t) MSB << 8 | LSB; // jump to whatever instruction is at the index: 0x(MSB)(LSB) in the array of uint8_t instructions.

            //         return 4; // takes 4 cycles
        }
}
