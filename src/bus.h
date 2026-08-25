#pragma once
#include "gbstructs.h"

void tick(GBMemory_t *mem);

uint8_t read_byte(GBMemory_t *mem, uint16_t addr); // 1 m cycle
uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State); // 1 m cycle.. incremements program counter
void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value); // 1 m cycle

void dma_update(GBMemory_t *mem);
void dma_read(GBMemory_t *mem, uint8_t value);
