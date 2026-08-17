#include "interrupt.h"

void request_interrupt(Interrupt_t inter, GBMemory_t *mem){
    switch(inter){
        case VBlank:
        {
            mem->IORegisters[0x0F] |= 1;
            break;
        }
        case LCD:
        {
            mem->IORegisters[0x0F] |= 2;
            break;
        }
        case Timer:
        {
            mem->IORegisters[0x0F] |= 4;
            break;
        }
        case Serial:
        {
            mem->IORegisters[0x0F] |= 8;
            break;
        }
        case Joypad:
        {
            mem->IORegisters[0x0F] |= 16;
            break;
        }
    }
}


Interrupt_t identify_interrupt(uint8_t IF){
    if(IF & 1){
        return VBlank;
    }else if(IF & 2){
        return LCD;
    }else if(IF & 4){
        return Timer;
    }else if(IF & 8){
        return Serial;
    }else if(IF & 16){
        return Joypad;
    }
    return NAI;
}


void process_interrupt(SharpState_t *CPU_State, GBMemory_t *mem){ // handles the interrupt at hand... lasts for 5 M-Cycles

    Interrupt_t inter = identify_interrupt(mem->IORegisters[0x0F]);

    switch(inter){
        case VBlank:
            mem->IORegisters[0x0F] &= 0x1E;
            CPU_State->PC = 0x40;
            break;
        case LCD:
            mem->IORegisters[0x0F] &= 0x1D;
            CPU_State->PC = 0x48;
            break;
        case Timer:
            mem->IORegisters[0x0F] &= 0x1B;
            CPU_State->PC = 0x50;
            break;
        case Serial:
            mem->IORegisters[0x0F] &= 0x17;
            CPU_State->PC = 0x58;
            break;
        case Joypad:
            mem->IORegisters[0x0F] &= 0x0F;
            CPU_State->PC = 0x60;
            break;
    }

    mem->IME = 0;

    /*
        same code for calling an instruction just gonna change it up a little bit in order to jummp to intterupt code
    */

    // wait for 2 m-cycles
    tick(&mem->sys_clock);
    tick(&mem->sys_clock);
    // push current pc onto the stack
    write_byte(mem, --CPU_State->SP, CPU_State->PC & 0xFF);
    write_byte(mem, --CPU_State->SP, (CPU_State->PC >> 8) & 0xFF); // put the bottom 8 bits on the stack
    // here's the final "generic fetch"
    tick(&mem->sys_clock);

}
