#include <string.h>

#include "bus.h"
#include "gbstructs.h"
#include "timers.h"


void tick(GBMemory_t *mem){
    mem->sys_clock++; // increment the cpu 1 m-cycle

    if(mem->CPU->state == 'w') {
        if(mem->sys_clock - mem->CPU->wm_cycle >= 2050){
            mem->CPU->state = 'n';
        }
        return;
    }

    dma_update(mem);
    timer_update(mem);

    int ppu_dots = mem->CPU->double_speed == 1 ? 2 : 4; // time keeping system of the PPU

    for(int i = 0; i < ppu_dots; i++){
        // ppu_update(mem);
    }

    // apu_update()
}

uint8_t read_byte(GBMemory_t *mem, uint16_t addr){
    if(addr < 0x8000){
        tick(mem);
        return mem->ROM[addr];
    }else if(addr < 0xA000){
        //VRAM
        tick(mem);
        return mem->VRAM[mem->IORegisters[0x4F] & 1][addr - 0x8000];
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        tick(mem);
        return mem->ExRAM[addr - 0xA000];
    }else if(addr < 0xD000){
        //WRAM
        tick(mem);
        return mem->WRAM0[addr - 0xC000];
    }else if(addr < 0xE000){
        //WRAM banks
        tick(mem);
        int bank = mem->IORegisters[0x70] & 0x7 == 0 ? 1 : mem->IORegisters[0x70] & 0x7;

        return mem->WRAMX[bank][addr - 0xD000];

    }else if(addr < 0xFE00){
        //Echo RAM (Mirror of WRAM .... im guessing bank 0 for right now...)
        tick(mem);
        return mem->WRAM0[addr - 0xE000];
    }else if(addr < 0xFEA0){
        //OAM
        tick(mem);
        return mem->OAM[addr - 0xFE00];
    }else if(addr < 0xFF00){
        //Forbidden memory (nintendo says u cant use this memory)
        tick(mem);
        return (uint8_t) 0;
    }else if(addr < 0xFF80){
        //IO Register
        tick(mem);

        switch(addr){ // handle the fact that i'm now making div a 16 bit register
            case 0xFF04:
            {
                return mem->DIV >> 8;
            }
            case 0xFF4D:
            {
                return mem->IORegisters[addr - 0xFF00] & 1;
            }
        }
        return mem->IORegisters[addr - 0xFF00];
    }else if(addr < 0xFFFF){
        tick(mem);

        return mem->HRAM[addr - 0xFF80];
    }else{
        //Interrupt Enable Register
        tick(mem);
        return mem->interrupt_enable;
    }

    // increment the system clock and update other peripherals
}


void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value){
    
    tick(mem);

    if(addr < 0x8000){
        ; //ROM
    }else if(addr < 0xA000){
        //VRAM
        mem->VRAM[mem->IORegisters[0x4F] & 1][addr - 0x8000] = value;
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        mem->ExRAM[addr - 0xA000] = value;
    }else if(addr < 0xD000){
        //WRAM
        mem->WRAM0[addr - 0xC000] = value;
    }else if(addr < 0xE000){
        //WRAM banks
        int bank = mem->IORegisters[0x70] & 0x7 == 0 ? 1 : mem->IORegisters[0x70] & 0x7;
        mem->WRAMX[bank][addr - 0xD000] = value;

    }else if(addr < 0xFE00){
        //Mirror of WRAM
        mem->WRAM0[addr - 0xE000] = value;
    }else if(addr < 0xFEA0){
        //OAM hold the attributes of objects / sprites
        // can only be written to duuring hblankad vblank periods
        mem->OAM[addr - 0xFE00] = value;
    }else if(addr < 0xFF00){
        ; //Forbidden memory
    }else if(addr < 0xFF80){
        if(addr == 0xFF01){
            mem->serial_output[mem->buffer_pointer++] = (char) value;
        }

        if(addr == 0xFF04){
            mem->DIV = 0;
        }else if(addr == 0xFF05 && mem->TIMA_overflowing){
            mem->TIMA_overflowing = 0; // cancel the interrupt and the writing to TIMA with TMA...
            mem->IORegisters[0x05] = value;
        }else if(addr == 0xFF05 && mem->TIMA_reloading){
            ; // ignore the write ! the value in TMA is going to be written to TIMA not whatever we passed in this function
        }else if(addr == 0xFF55 && !mem->vdma.active){
            mem->IORegisters[addr - 0xFF00] = value;
            mem->vdma.active = 1;
        }else{
            mem->IORegisters[addr - 0xFF00] = value;
        }
    }else if(addr < 0xFFFF){
        //HRAM
        mem->HRAM[addr - 0xFF80] = value;
    }else{
        mem->interrupt_enable = value;
    }


    return;
}

uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State){
    /*
        returns opcode and increments program counter
    */
    return read_byte(mem, CPU_State->PC++);
}

uint8_t raw_read_byte(GBMemory_t *mem, uint8_t addr){ // reads a byte from memory without taking up an m-cycle
    if(addr < 0x8000){
        return mem->ROM[addr];
    }else if(addr < 0xA000){
        //VRAM
        return mem->VRAM[mem->IORegisters[0x4F] & 1][addr - 0x8000];
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        return mem->ExRAM[addr - 0xA000];
    }else if(addr < 0xD000){
        //WRAM
        return mem->WRAM0[addr - 0xC000];
    }else if(addr < 0xE000){
        //WRAM banks
        int bank = mem->IORegisters[0x70] & 0x7 == 0 ? 1 : mem->IORegisters[0x70] & 0x7;

        return mem->WRAMX[bank][addr - 0xD000];

    }else if(addr < 0xFE00){
        //Echo RAM (Mirror of WRAM .... im guessing bank 0 for right now...)
        return mem->WRAM0[addr - 0xE000];
    }else if(addr < 0xFEA0){
        //OAM
        return mem->OAM[addr - 0xFE00];
    }else if(addr < 0xFF00){
        //Forbidden memory (nintendo says u cant use this memory)
        return (uint8_t) 0;
    }else if(addr < 0xFF80){
        //IO Register

        switch(addr){ // handle the fact that i'm now making div a 16 bit register
            case 0xFF04:
            {
                return mem->DIV >> 8;
            }
            case 0xFF4D:
            {
                return mem->IORegisters[addr - 0xFF00] & 1;
            }
        }
        return mem->IORegisters[addr - 0xFF00];
    }else if(addr < 0xFFFF){

        return mem->HRAM[addr - 0xFF80];
    }else{
        //Interrupt Enable Register
        return mem->interrupt_enable;
    }
}

void dma_update(GBMemory_t *mem){

    mem->vdma.source =  (((uint16_t) mem->IORegisters[0x51]) << 8  | (mem->IORegisters[0x52] & 0xF0));
    mem->vdma.dest =0x8000 | ((uint16_t)(mem->IORegisters[0x53] & 0x1F) << 8) | (mem->IORegisters[0x54] & 0xF0);

    if (!mem->vdma.active) return;

    if(mem->vdma.hblank_mode){
        // need the PPU for dih wun rih here
        return;
    }

    int bytes_this_cycle;

    if(mem->CPU->double_speed){
        bytes_this_cycle = 1; // double speed
    }else{
        bytes_this_cycle = 2; // normal speed
    }

    for(int i = 0; i < bytes_this_cycle && mem->vdma.bytes_remaining; i++){
        uint8_t value = raw_read_byte(mem, mem->vdma.source);

        uint8_t bank = mem->IORegisters[0x4F] & 1;

        mem->VRAM[bank][mem->vdma.dest - 0x8000] = value;

        mem->vdma.source++;
        mem->vdma.dest++;
        mem->vdma.bytes_remaining--;
    }

    if(mem->vdma.bytes_remaining == 0){
        mem->vdma.active = 0;
        mem->IORegisters[0x55] = 0xFF;
    }
}
