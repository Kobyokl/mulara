#pragma once
#include "cpu_core.h"

typedef enum { VBlank, LCD, Timer, Serial, Joypad, NAI } Interrupt_t; // NAI = not an interrupt

Interrupt_t identify_interrupt(uint8_t IF);

void request_interrupt(Interrupt_t inter, GBMemory_t *mem);
void process_interrupt(SharpState_t *CPU_State, GBMemory_t *mem);
