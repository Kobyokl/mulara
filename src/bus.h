// header file that prototypes functions that emulate bus behavior
#include <stdio.h>
#include <stdint.h>
#include <gbstructs.h>

#pragma once 

uint8_t read_byte(GBMemory_t *mem, uint16_t addr);
uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State);

void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value);