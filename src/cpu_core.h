#pragma once

#include "bus.h"

uint8_t half_carry_flagged(); // checks if the half carry flag should be set or naw
void sm83_instruction_handler(GBMemory_t *mem, SharpState_t *CPU_State); //handles all cpu instructions


