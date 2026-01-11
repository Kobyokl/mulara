#include "bus.h"

uint8_t half_carry_16bit_add(uint16_t num1, uint16_t num2){
    // check if bit 11 carried into bit 12
    return (uint8_t) (((num1 & 0x0FFF) + (num2 & 0x0FFF)) & 0x1000) == 0x1000; 
}
uint8_t half_carry_16bit_sub(uint16_t num1, uint16_t num2) {
    // returns 1 if half carry occured with subtraction...
    return (uint8_t)((num1 & 0x0FFF) < (num2 & 0x0FFF));
}

uint8_t half_carry_8bit_add(uint8_t num1, uint8_t num2){
    // check if bit 3 carried into bit 4 
    return (uint8_t) (((num1 & 0x0F) + (num2 & 0x0F) & 0x10) == 0x010); 
}
uint8_t half_carry_8bit_sub(uint8_t num1, uint8_t num2) {
    // Half-borrow occurs if lower nibble of num1 is smaller than num2
    return (uint8_t)((num1 & 0x0F) < (num2 & 0x0F));
}




void sm83_instruction_handler(GBMemory_t *mem, SharpState_t *CPU_State){

       uint8_t opcode = fetch_byte(mem, CPU_State);

       switch(opcode){
        case(0x00):
            break; 

        case(0x01):
        {
            uint8_t LSB = fetch_byte(mem, CPU_State);
            uint8_t MSB = fetch_byte(mem, CPU_State);
            CPU_State->BC = (uint16_t) MSB << 8 | LSB;
            break; 
        }
        
        case(0x02): 
        {
            CPU_State->BC = (uint16_t) CPU_State->A ; 
            break; 
        }
            
        case(0x03): 
        {
            CPU_State->BC++; 
            break;
        }

        case(0x04):
        {
            uint8_t B = (uint8_t)(CPU_State->BC >> 8);

            CPU_State->F &= 0x10; // clear all but carry

            CPU_State->F |= half_carry_8bit(B , 1) << 5; //half carry flag
            CPU_State->F &= 0b10111111;  // negative flag (bit 6) is unset

            B++; 

            if(B == 0){ // zero flag
                CPU_State->F |= 0x80;
            }
            
            
            CPU_State->BC = (CPU_State->BC & 0x00FF) | (B << 8); //clears B and sets it to our new incremented B
            break; 
        }
            
        case(0x05):
        {
            uint8_t B = (uint8_t)(CPU_State->BC >> 8);

            CPU_State->F &= 0x10;

            CPU_State->F |= half_carry_8bit_sub(B, 1) << 5; // H
            CPU_State->F |= 0b01000000; 

            B--;

            if (B == 0){
                CPU_State->F |= 0x80; // Z
            }

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (B << 8); 
            break;
        }

        case(0x06): 
            uint8_t B = fetch_byte(mem, CPU_State); 

            CPU_State->F &= 0xF0; //leave states unmodified 

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (B << 8); 
            break;


        case(0xC3):
                uint8_t LSB = fetch_byte(mem, CPU_State);
                uint8_t MSB = fetch_byte(mem, CPU_State);
                CPU_State->PC = (uint16_t) MSB << 8 | LSB; // jump to whatever instruction is at the index: 0x(MSB)(LSB) in the array of uint8_t instructions.
        }
}


