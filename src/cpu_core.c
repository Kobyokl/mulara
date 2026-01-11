#include "bus.h"

void sm83_instruction_handler(GBMemory_t *mem, SharpState_t *CPU_State){

       uint8_t opcode = fetch_byte(mem, CPU_State);

        switch(opcode){
            case(0x00): 
    
                return; 
            case(0x01):
                
                uint8_t LSB = fetch_byte(mem, CPU_State);
                uint8_t MSB = fetch_byte(mem, CPU_State);
                CPU_State->BC = (uint16_t) MSB << 8 | LSB;

            case(0x02): 
                CPU_State->BC = (uint16_t) CPU_State->A ; 

            case(0x03): 
                CPU_State->BC++; 

            case(0x04): 
                CPU_State->BC += 1 << 8; 
                if(~CPU_State->BC){ // zero flag
                    CPU_State->F |= 0x80;
                }

                CPU_State->F &= 0xBF; // negative flag (bit 6)

            case(0xC3):
                    uint8_t LSB = fetch_byte(mem, CPU_State);
                    uint8_t MSB = fetch_byte(mem, CPU_State);
                    CPU_State->PC = (uint16_t) MSB << 8 | LSB; // jump to whatever instruction is at the index: 0x(MSB)(LSB) in the array of uint8_t instructions.
        }
}


