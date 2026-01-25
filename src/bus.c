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
        //IO Register
        return 0x01;
    }else if(addr < 0xFFFF){
        return mem->HRAM[addr - 0xFF80];
    }else{
        //Interrupt Enable Register
        //idrk what to do
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
    return read_byte(mem, CPU_State->PC++);
}
