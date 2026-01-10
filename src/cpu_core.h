#include <stdio.h>
#include <stdint.h>
#include <gbstructs.h>

uint8_t read_byte(GBMemory_t *mem, uint16_t addr);
void write_byte(GBMemory_t *mem, uint16_t addr, uint8_t value);
uint8_t fetch_byte(GBMemory_t *mem, SharpState_t *CPU_State);

void sm83_instructionHandler(GBMemory_t *mem, SharpState_t *CPU_State);
