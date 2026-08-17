#pragma once
#include "gbstructs.h"

uint8_t read_byte(GBMemory_t *mem, uint16_t addr); // 1 m cycle
uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State); // 1 m cycle.. incremements program counter

void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value); // 1 m cycle
