#include "bus.h"
#include "interrupt.h"
#include "timers.h"


void timer_update(GBMemory_t *mem){

    if(mem->TIMA_overflowing){ // update the TIMA register on the next M-cycle
        mem->TIMA_reloading = 1;
        mem->IORegisters[0x05] = mem->IORegisters[0x06];
        mem->TIMA_overflowing = 0;
        request_interrupt(Timer, mem);

    }else{
        mem->TIMA_reloading = 0;
    }

    if(DIV_handler(mem)){
        TIMA_handler(mem);
    }

}

int DIV_handler(GBMemory_t *mem){ // increments the div register, handles timer behavior, and is called upon every m-cycle tick

    mem->DIV += 4;

    int ret;

    uint8_t TAC_enabled = (mem->IORegisters[0x07] >> 2) & 1;
    uint8_t clock_sel = mem->IORegisters[0x07] & 3;
    uint8_t sel_bit; // the bit that was selected from the DIV register

    switch(clock_sel){
        case 0: sel_bit = (mem->DIV >> 9) & 1; break;
        case 1: sel_bit = (mem->DIV >> 3) & 1;; break;
        case 2: sel_bit = (mem->DIV >> 5) & 1;; break;
        case 3: sel_bit = (mem->DIV >> 7) & 1;; break;
    }

    mem->and_result = sel_bit & TAC_enabled;

    if(mem->prev_and_result > mem->and_result){ // falling edge detection
        ret = 1;
    } else{
        ret = 0;
    }

    mem->prev_and_result = mem->and_result; // update prev_and_result to check for the next falling edge

    return ret;

}


void TIMA_handler(GBMemory_t *mem){
    // now lets hadnle the incrementing of the TIMA register

    if( !(++mem->IORegisters[0x05]) ){
        // the reloading of this register and the interrupt can be canceleld by writing to the TIMA register
        // between the two M-cycles that this function may be called on (between one of the 4 T-cycles)
        mem->TIMA_overflowing = 1;
    }
}
