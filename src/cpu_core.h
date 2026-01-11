#pragma once

#include "bus.h"

uint8_t half_carry_16bit(uint16_t num1, uint16_t num2); // checks if there's a half carry flag  between 16 bit operands
uint8_t half_carry_16bit_sub(uint16_t num1, uint16_t num2); // similar thing but w/ subtraction 
uint8_t half_carry_8bit(uint8_t num1, uint8_t num2); // checks if there's a half carry flag  between 8 bit operands
uint8_t half_carry_8bit_sub(uint8_t num1, uint8_t num2); // similar thing but w/ subtraction

void sm83_instruction_handler(GBMemory_t *mem, SharpState_t *CPU_State); //handles SharpSM83 Instruction Set


