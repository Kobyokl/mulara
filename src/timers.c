#include "cpu_core.h"
#include "interrupt.h"


/*


      CHANGE THIS CODE LATER SO THAT YOU PROPERLY HANDLE THE OBSCURE TIMER BEHAVIOR AND SHIIIII LIKEEEE THATTTT




*/

void timer_update(SharpState_t *CPU_State, GBMemory_t *mem){
    DIV_handler(CPU_State, mem);
    TIMA_handler(CPU_State, mem);
}

void DIV_handler(SharpState_t *CPU_State, GBMemory_t *mem){
    if(!(mem->sys_clock % 17556)){ // handles incrementing of the div register...
        mem->IORegisters[0x04]++;
    }
}

void TIMA_handler(SharpState_t *CPU_State, GBMemory_t *mem){
    // now lets hadnle the incrementing of the TIMA register

    uint8_t TAC = mem->IORegisters[0x07];

    unsigned int enable = (TAC >> 2) & 1;
    unsigned int clock_select = TAC & 3;

    if(enable){
        switch(clock_select){
            case 0:
            {
                if(!(mem->sys_clock % 256)){
                    if(++mem->IORegisters[0x05]){
                        mem->IORegisters[0x05] = mem->IORegisters[0x06];
                        request_interrupt(Timer, mem);
                    }
                }
                break;
            }
            case 1:
            {
                if(!(mem->sys_clock % 4)){
                    if(!(++mem->IORegisters[0x05])){
                        mem->IORegisters[0x05] = mem->IORegisters[0x06];
                        request_interrupt(Timer, mem);
                    }
                }
                break;
            }
            case 2:
            {
                if(!(mem->sys_clock % 16)){
                    if(!(++mem->IORegisters[0x05])){
                        mem->IORegisters[0x05] = mem->IORegisters[0x06];
                        request_interrupt(Timer, mem);
                    }
                }
                break;
            }
            case 3:
            {
                if(!(mem->sys_clock % 64)){
                    if(!(++mem->IORegisters[0x05])){
                        mem->IORegisters[0x05] = mem->IORegisters[0x06];
                        request_interrupt(Timer, mem);
                    }
                }
            }
        }
    }
}
