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

       uint8_t opcode = fetch_byte(mem, CPU_State); // 1 m cycle

       switch(opcode){
        case(0x00):
            break;

        case(0x10):
            CPU_State->state = 's';

        /*
        ========================================================================================================
                                            8-bit Load/Store/Move
        ========================================================================================================
        */

        case(0x02):
        {
            write_byte(mem, CPU_State->BC, CPU_State->A); // 1 m cycle
            break;
        }
        case(0x12):
        {
            write_byte(mem, CPU_State->DE, CPU_State->A); // 1 m cycle
            break;
        }
        case(0x22):
        {
            write_byte(mem, CPU_State->HL, CPU_State->A); // 1 m cycle
            CPU_State->HL++;
            break;
        }
        case(0x32):
        {
            write_byte(mem, CPU_State->HL, CPU_State->A); // 1 m cycle
            CPU_State->HL--;
            break;
        }
        case(0x42):
        {
            uint8_t D = (CPU_State->DE & 0xFF00) >> 8;

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (D << 8);
            break;
        }
        case(0x52):
        {
            // a debug instruction
            uint8_t D = (CPU_State->DE & 0xFF00) >> 8;

            CPU_State->DE = (CPU_State->DE & 0x00FF) | (D << 8);
            break;
        }
        case(0x62):
        {

            uint8_t D = (CPU_State->DE & 0xFF00) >> 8;

            CPU_State->HL = (CPU_State->HL & 0x00FF) | (D << 8);
            break;
        }
        case(0x63):
        {

            uint8_t D = (CPU_State->DE & 0xFF00) >> 8;

            write_byte(mem, CPU_State->HL, D);

            break;
        }

        case(0x06):
        {
            uint8_t newB = fetch_byte(mem, CPU_State); // 1 m cycle

            CPU_State->F &= 0xFF; //leave states unmodified

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (newB << 8);
            break;
        }
        case(0x16):
        {
            uint8_t newD = fetch_byte(mem, CPU_State); // 1 m cycle

            CPU_State->F &= 0xFF; //leave states unmodified

            CPU_State->DE = (CPU_State->BC & 0x00FF) | (newD << 8);
            break;
        }
        case(0x26):
        {
            uint8_t newH = fetch_byte(mem, CPU_State); // 1 m cycle

            CPU_State->F &= 0xFF; //leave states unmodified

            CPU_State->HL = (CPU_State->BC & 0x00FF) | (newH << 8);
            break;
        }
        case(0x36):
        {
            uint8_t value = fetch_byte(mem, CPU_State); // 1 m cycle

            CPU_State->F &= 0xFF; //leave states unmodified

            write_byte(mem, CPU_State->HL, value); // 1 m cycl
            break;
        }
        case(0x46):
        {
            uint8_t value = read_byte(mem, CPU_State->HL);

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (value << 8);
            break;
        }
        case(0x56):
        {
            uint8_t value = read_byte(mem, CPU_State->HL);

            CPU_State->DE = (CPU_State->DE & 0x00FF) | (value << 8);
            break;
        }
        case(0x66):
        {
            uint8_t value = read_byte(mem, CPU_State->HL);

            CPU_State->HL = (CPU_State->HL & 0x00FF) | (value << 8);
            break;
        }
        case(0x76):
        {
            CPU_State->state = 'h';
            break;
        }

        case(0x40):
        {
            uint8_t B = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (B << 8);
            break;
        }
        case(0x50):
        {
            uint8_t B = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->DE = (CPU_State->DE & 0x00FF) | (B << 8);
            break;
        }
        case(0x60):
        {
            uint8_t B = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->HL = (CPU_State->HL & 0x00FF) | (B << 8);
            break;
        }
        case(0x70):
        {
            uint8_t B = (CPU_State->BC & 0xFF00) >> 8;

            write_byte(mem, CPU_State->HL, B);
            break;
        }

        case(0x41):
        {
            uint8_t C = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (C << 8);
            break;
        }
        case(0x51):
        {
            uint8_t C = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->DE = (CPU_State->DE & 0x00FF) | (C << 8);
            break;
        }
        case(0x61):
        {
            uint8_t C = (CPU_State->BC & 0xFF00) >> 8;

            CPU_State->DE = (CPU_State->DE & 0x00FF) | (C << 8);
            break;
        }
        case(0x71):
        {
            uint8_t C = (CPU_State->BC & 0xFF00) >> 8;

            write_byte(mem, CPU_State->HL, C);
            break;
        }


        case(0x0A):
        {
            uint8_t deref_BC = read_byte(mem, CPU_State->BC);

            CPU_State->A = deref_BC;
        }
        case(0x1A):
        {
            uint8_t deref_DE = read_byte(mem, CPU_State->DE);

            CPU_State->A = deref_DE;
        }
        case(0x2A):
        {
            uint8_t deref_HL = read_byte(mem, CPU_State->HL);

            CPU_State->A = deref_HL;
            CPU_State->HL++;
        }
        case(0x3A):
        {
            uint8_t deref_HL = read_byte(mem, CPU_State->HL);

            CPU_State->A = deref_HL;
            CPU_State->HL--;
        }


        /*
        ========================================================================================================
                                            16-bit Load/Store/Move
        ========================================================================================================
        */

        case(0x01):
        {
            uint8_t LSB = fetch_byte(mem, CPU_State);
            uint8_t MSB = fetch_byte(mem, CPU_State);
            CPU_State->BC = (uint16_t) MSB << 8 | LSB;
            break;
        }

        case(0x08):
        {
            uint8_t LSB = fetch_byte(mem, CPU_State);
            uint8_t MSB = fetch_byte(mem, CPU_State);
            uint16_t nn = (MSB << 8) | LSB;

            write_byte(mem, nn, (uint8_t) (CPU_State->SP & 0x00FF)); //write lower bits first
            write_byte(mem, nn + 1, (uint8_t) ((CPU_State->SP & 0xFF00) >> 8));  //write higher bits last
            break;
        }

        /*
        ========================================================================================================
                                            8-bit Arithmetic Logic Unit
        ========================================================================================================
        */

        case(0x03):
        {
            CPU_State->BC++; // 1 m cycle
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

            CPU_State->F |=  _sub(B, 1) << 5; // H
            CPU_State->F |= 0b01000000;

            B--;

            if (B == 0){
                CPU_State->F |= 0x80; // Z
            }

            CPU_State->BC = (CPU_State->BC & 0x00FF) | (B << 8);
            break;
        }



        case(0x07):
            CPU_State->F &= 0; //unset all other flags

            if(CPU_State->A & 0x80){
                CPU_State->F |= 0b00010000; // set the carry flag
                CPU_State->A = (CPU_State->A << 1) + 1;
            }else{
                CPU_State->A <<= 1;
            }
            break;


        case(0x09):


        case(0xC3):
                uint8_t LSB = fetch_byte(mem, CPU_State);
                uint8_t MSB = fetch_byte(mem, CPU_State);
                CPU_State->PC = (uint16_t) MSB << 8 | LSB; // jump to whatever instruction is at the index: 0x(MSB)(LSB) in the array of uint8_t instructions.
        }
}
