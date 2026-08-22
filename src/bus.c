#include "bus.h"
#include "gbstructs.h"
#include "timers.h"


void tick(GBMemory_t *mem){
    mem->sys_clock++; // increment our system clock (1 m-cycle)

    timer_update(mem);
    // ppu_update()
    // apu_update()

}

uint8_t read_byte(GBMemory_t *mem, uint16_t addr){
    if(addr < 0x8000){
        tick(mem);
        return mem->ROM[addr];
    }else if(addr < 0xA000){
        //VRAM
        tick(mem);
        return mem->VRAM[addr - 0x8000];
    }else if(addr < 0xC000){
        //ExRAM (on cartridge)
        tick(mem);
        return mem->ExRAM[addr - 0xA000];
    }else if(addr < 0xE000){
        //WRAM
        tick(mem);
        return mem->WRAM[addr - 0xC000];
    }else if(addr < 0xFE00){
        //Echo RAM (Mirror of WRAM)
        tick(mem);
        return mem->WRAM[addr - 0xE000];
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
        if(addr >= 0xC752 && addr <= 0xC756){
            // printf(
            //     "\nWATCHPOINT: PC=%04X wrote %02X to %04X\n",
            //     mem->CPU->PC,
            //     value,
            //     addr
            // );

            // printf(
            //     "AF:%02X%02X BC:%04X DE:%04X HL:%04X SP:%04X\n",
            //     mem->CPU->A,
            //     mem->CPU->F,
            //     mem->CPU->BC,
            //     mem->CPU->DE,
            //     mem->CPU->HL,
            //     mem->CPU->SP
            // );
        }
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
        if(addr == 0xFF01){
            mem->serial_output[mem->buffer_pointer++] = (char) value;
        }


        if(addr == 0xFF04){
            mem->DIV = 0;
        }else if(addr == 0xFF05 && mem->TIMA_overflowing){
            mem->TIMA_overflowing = 0; // cancel the interrupt and the writing to TIMA with TMA...
        }else if(addr == 0xFF05 && mem->TIMA_reloading){
            ; // ignore the write ! the value in TMA is going to be written to TIMA not whatever we passed in this function
        }else{
            mem->IORegisters[addr - 0xFF00] = value;
        }
    }else if(addr < 0xFFFF){
        //HRAM
        mem->HRAM[addr - 0xFF80] = value;
    }else{
        mem->interrupt_enable = value;
        printf("IO WRITE: FFFF = %02X\n", value);
    }

    tick(mem);

    return;
}

uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State){
    /*
        returns opcode and increments program counter
    */
    return read_byte(mem, CPU_State->PC++);
}
