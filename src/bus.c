#include "gbstructs.h"


void tick(unsigned int *sys_clock){
    (*sys_clock)++; // increment our system clock

    // timer_update()
    // ppu_update()
    // apu_update()

}

uint8_t read_byte(GBMemory_t *mem, uint16_t addr){
    if(addr < 0x8000){
        tick(&mem->sys_clock);
        return mem->ROM[addr];
    }else if(addr < 0xA000){
        //VRAM
        tick(&mem->sys_clock);
        return mem->VRAM[addr - 0x8000];
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        tick(&mem->sys_clock);
        return mem->ExRAM[addr - 0xA000];
    }else if(addr < 0xE000){
        //WRAM
        tick(&mem->sys_clock);
        return mem->WRAM[addr - 0xC000];
    }else if(addr < 0xFE00){
        //Echo RAM (Mirror of WRAM)
        tick(&mem->sys_clock);
        return mem->WRAM[addr - 0xE000];
    }else if(addr < 0xFEA0){
        //OAM
        tick(&mem->sys_clock);
        return mem->OAM[addr - 0xFE00];
    }else if(addr < 0xFF00){
        //Forbidden memory (nintendo says u cant use this memory)
        tick(&mem->sys_clock);
        return (uint8_t) 0;
    }else if(addr < 0xFF80){
        //IO Register
        tick(&mem->sys_clock);
        return mem->IORegisters[addr - 0xFF00];
    }else if(addr < 0xFFFF){
        tick(&mem->sys_clock);
        return mem->HRAM[addr - 0xFF80];
    }else{
        //Interrupt Enable Register
        tick(&mem->sys_clock);
        return mem->interrupt_enable;
    }

    // increment the system clock and update other peripherals
}

void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value){
    if(addr < 0x8000){
        ; //ROM
    }else if(addr < 0xA000){
        //VRAM
        mem->VRAM[addr - 0x8000] = value;
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        mem->ExRAM[addr - 0xA000] = value;
    }else if(addr < 0xE000){
        //WRAM
        mem->WRAM[addr - 0xC000] = value;
    }else if(addr < 0xFE00){
        //Mirror of WRAM
        mem->WRAM[addr - 0xE000] = value;
    }else if(addr < 0xFEA0){
        //OAM
        mem->OAM[addr - 0xFE00] = value;
    }else if(addr < 0xFF00){
        ; //Forbidden memory
    }else if(addr < 0xFF80){
        if(addr == 0xFF04){
            mem->IORegisters[0x04] = 0;
        }else{
            mem->IORegisters[addr - 0xFF00] = value;
        }
    }else if(addr < 0xFFFF){
        //HRAM
        mem->HRAM[addr - 0xFF80] = value;
    }else{
        mem->interrupt_enable = value;
    }

    tick(&mem->sys_clock);

    return;
}

uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State){
    /*
        returns opcode and increments program counter
    */
    return read_byte(mem, CPU_State->PC++);
}
