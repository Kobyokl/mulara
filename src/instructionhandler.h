#pragma once

#include "bus.h"

uint8_t block_finder(uint8_t opcode);

uint8_t get_condition(SharpState_t *CPU, uint8_t identifier);

uint16_t set_16bit_reg(SharpState_t *CPU, uint16_t value, uint8_t identifier);
uint16_t get_16bit_reg(SharpState_t *CPU, uint8_t identifier);

uint8_t set_8bit_reg(GBMemory_t *mem, SharpState_t *CPU, uint8_t value, uint8_t identifier);
uint8_t get_8bit_reg(GBMemory_t *mem, SharpState_t *CPU, uint8_t identifier);

uint16_t set_16bitstk(SharpState_t *CPU, uint8_t identifier, uint16_t value);
uint16_t get_16bitstk(SharpState_t *CPU, uint8_t identifier);

void better_instruction_handler(SharpState_t *CPU_State, GBMemory_t *mem); 
