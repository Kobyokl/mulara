#include "bus.h"

extern const uint8_t gb_daa_table[256 * 8 * 2]; // In file daa_table.c

/*
    to be clear none of these functions take any machine cycles... only the ones in bus.h do
*/

uint8_t block_finder(uint8_t opcode) { // return value 0 - 3
    return (opcode & 0b11000000) >> 6;
}

uint8_t get_condition(SharpState_t *CPU, uint8_t identifier){
    switch(identifier){
        case (0): return (CPU->F & 0x80) == 0; // nz
        case (1): return (CPU->F & 0x80) != 0; // z
        case (2): return (CPU->F & 0x01) == 0; // nc
        case (3): return (CPU->F & 0x01) != 0; // c
    }
    return 0; 
}

uint16_t set_16bit_reg(SharpState_t *CPU, uint16_t value, uint8_t identifier){
    switch(identifier){
        case(0): CPU->BC = value; return CPU->BC; 
        case(1): CPU->DE = value; return CPU->DE; 
        case(2): CPU->HL = value; return CPU->HL; 
        case(3): CPU->SP = value; return CPU->SP; 
    }
    return 0; 
}

uint16_t get_16bit_reg(SharpState_t *CPU, uint8_t identifier){
    switch(identifier){
        case(0): return CPU->BC;
        case(1): return CPU->DE;
        case(2): return CPU->HL;
        case(3): return CPU->SP;
    }
    return 0;
    
}

uint8_t set_8bit_reg(GBMemory_t *mem, SharpState_t *CPU, uint8_t value, uint8_t identifier){
    switch(identifier){
        case(0): CPU->BC = (CPU->BC & 0x00FF) | (value << 8); return CPU->BC >> 8; // b
        case(1): CPU->BC |= (uint16_t) value; return CPU->BC & 0x00FF; // c
        case(2): CPU->DE = (CPU->DE & 0x00FF) | (value << 8); return CPU->DE >> 8; // d
        case(3): CPU->DE |= (uint16_t) value; return CPU->DE & 0x00FF; // e
        case(4): CPU->HL = (CPU->HL & 0x00FF) | (value << 8); return CPU->HL >> 8; // h
        case(5): CPU->HL |= (uint16_t) value; return CPU->HL & 0x00FF; // l
        case(6): write_byte(mem, CPU->HL, value); return 0; // [hl] (uses an m-cycle) returns zero by defualt so we have to handle some instructions here seperately
        case(7): CPU->A = value; return CPU->A;// a
    }
    return 0; 
}

uint8_t get_8bit_reg(GBMemory_t *mem, SharpState_t *CPU, uint8_t identifier){
    switch(identifier){
        case(0): return CPU->BC >> 8;          // b
        case(1): return CPU->BC & 0xFF;        // c
        case(2): return CPU->DE >> 8;          // d
        case(3): return CPU->DE & 0xFF;        // e
        case(4): return CPU->HL >> 8;          // h
        case(5): return CPU->HL & 0xFF;        // l
        case(6): return read_byte(mem, CPU->HL); // [hl] uses mem cycle
        case(7): return CPU->A;                // a
    }
    return 0; 
}

void better_instruction_handler(SharpState_t *CPU_State, GBMemory_t *mem){
    uint8_t opcode = fetch_byte(mem, CPU_State);
    uint8_t block = block_finder(opcode);

    switch(opcode){ 
        case(0): return; 
        case(16):
        {
            CPU_State->state = 's';
            fetch_byte(mem, CPU_State);
            return; 
        }
        case(0b01110110):
        {
            CPU_State->state = 'h'; 
            return; 
        }
        case(0b00011000):
        {
            //JR n16 takes 3 m-cycles
            CPU_State->PC += (char) fetch_byte(mem, CPU_State);
            read_byte(mem, 0); 
            return;
        }

        /*
            Rotate, shift, and bit operation instructions
        */

        case(0b00000111): 
        {
            CPU_State->F &= 0; //unset all other flags

            if(CPU_State->A & 0x80){
                CPU_State->F |= 0b00010000; // set the carry flag
                CPU_State->A = (CPU_State->A << 1) + 1;
            }else{
                CPU_State->A <<= 1;
            }
            return;
        }
        case(0b00001111): 
        {
            CPU_State->F &= 0; //unset all other flags
            if(CPU_State->A & 1){ 
                CPU_State->F |= 0b00010000; 
                CPU_State->A = (CPU_State->A >> 1) | 0x80;
            }else{
                CPU_State->A >>= 1;
            }
            return; 
        }
        case(0b00010111): 
        {
            uint8_t carry = (CPU_State->A & 0x80) >> 7;
            CPU_State->A = (CPU_State->A << 1) | ((CPU_State->F & 0x10) >> 4); // just like 0x07 but bit zero is dependent on the carry flag BEFORE the flag is modified...
            CPU_State->F = carry << 4; // only carry flag
            return;  
        }
        case(0b00011111): 
        {
            uint8_t carry = (CPU_State->A & 0x01);
            CPU_State->A = (CPU_State->A >> 1) | (CPU_State->F & 0x10); // just like 0x07 but bit zero is dependent on the carry flag BEFORE the flag is modified...
            CPU_State->F = carry << 4; // only carry flag
            return;  
        }
        case(0b00100111): 
        {
            // this was taken from someone else's emulator icl

            uint32_t temp = (((uint32_t)CPU_State->A) << (3 + 1))
                        | ((((uint32_t)CPU_State->F >> 4) & 7) << 1);
            CPU_State->A = gb_daa_table[temp];
            CPU_State->F = gb_daa_table[temp + 1];
            GB_CPUClockCounterAdd(4);
            return;
        }
        case(0b00101111): 
        {
            CPU_State->A = ~CPU_State->A; 
            CPU_State->F |= 0b01100000;
            return;
        }
        case(0b00110111): 
        {
            CPU_State->F |= 0b00010000; 
            CPU_State->F &= 0b10011111; 
            return;
        }
        case(0b00111111): 
        {
            CPU_State->F ^= 0b00010000;
            CPU_State->F &= 0b10011111;
            return; 
        }

    switch(block){
        case(0): // block 0
        {
            switch(opcode & 0x07){ // check the last 3 bits of the opcode
                case(4): // inc r8
                {
                    uint8_t identifier = (opcode & 0b00111000) >> 3;
                                    
                    uint8_t old = get_8bit_reg(mem, CPU_State, identifier);
                    uint8_t regvalue = old + 1;
                                    
                    set_8bit_reg(mem, CPU_State, regvalue, identifier); 

                    CPU_State->F &= 0x10; // preserve carry
                    if ((old & 0x0F) == 0x0F)
                        CPU_State->F |= 0x20; // half carry 
                    if(regvalue == 0)
                        CPU_State->F |= 0x80; // zero
                    CPU_State->F &= 0b10111111; // unset the n flag

                    return;
                }


                case(5): // dec r8
                {
                    uint8_t identifier = (opcode & 0b00111000) >> 3;
                                    
                    uint8_t old = get_8bit_reg(mem, CPU_State, identifier);
                    uint8_t regvalue = old - 1;
                                    
                    set_8bit_reg(mem, CPU_State, regvalue, identifier); 

                    CPU_State->F &= 0x10; // preserve carry
                    CPU_State->F |= 0b01000000;
                    if ((old & 0x0F) == 0x00)
                        CPU_State->F |= 0x20; // half carry 
                    if(regvalue == 0)
                        CPU_State->F |= 0x80; // zero
                    return;
                }
                case(6): // ld r8 immediate
                {
                    uint8_t value = fetch_byte(mem, CPU_State); 
                    uint8_t identifier = (opcode & 0b00111000) >> 3;

                    if(identifier == 6){ // handle [hl] 
                        write_byte(mem, CPU_State->HL, value); 
                        return; 
                    }

                    set_8bit_reg(mem, CPU_State, value, identifier); 
                    return; 
                }
                case(0): // jr cond, imm8
                {
                    uint8_t condition = (opcode & 0b00011000) >> 3; 
                    char offset = (char) fetch_byte(mem, CPU_State); 
                    if(get_condition(CPU_State, condition)){
                        CPU_State->PC += offset;
                        read_byte(mem, 0); 
                    }
                    return;
                }
            }
            switch(opcode & 0x0F){
                case(1): // ld r16, imm16
                {
                    uint8_t valueLSB = fetch_byte(mem, CPU_State); 
                    uint8_t valueMSB = fetch_byte(mem, CPU_State); 
                    uint8_t identifier = (opcode & 0b00110000) >> 4;

                    set_16bit_reg(CPU_State, (valueMSB << 8) | valueLSB, identifier);
                    return;
                }
                case(2): // ld [r16mem], a
                {
                    uint8_t identifier = (opcode & 0b00110000) >> 4;

                    if(identifier == 2){
                        write_byte(mem, CPU_State->HL, CPU_State->A); 
                        CPU_State->HL++;
                        return; 
                    }else if(identifier == 3){ 
                        write_byte(mem, CPU_State->HL, CPU_State->A); 
                        CPU_State->HL--;
                        return;
                    }
                    write_byte(mem, get_16bit_reg(CPU_State, identifier), CPU_State->A); 
                    return;
                }
                case(10): // ld a, [r16mem]
                {
                    uint8_t identifier = (opcode & 0b00110000) >> 4; 
                    uint8_t value; 

                    if(identifier == 2){
                        value = read_byte(mem, CPU_State->HL); 
                        CPU_State->HL++;
                    }else if(identifier == 3){ 
                        value = read_byte(mem, CPU_State->HL); 
                        CPU_State->HL--;
                    }else{
                        value = read_byte(mem, get_16bit_reg(CPU_State, identifier)); 
                    }

                    CPU_State->A = value;

                    return;
                }
                case(8): // ld [imm16], sp
                { 
                    uint8_t addrLSB = fetch_byte(mem, CPU_State); 
                    uint8_t addrMSB = fetch_byte(mem, CPU_State);  

                    uint16_t addr = (addrMSB << 8) | addrLSB; 

                    write_byte(mem, addr, CPU_State->SP & 0xFF);
                    write_byte(mem, addr + 1, (CPU_State->SP >> 8)); 

                    return; 
                }
                case(3): // inc r16
                {
                    uint8_t identifier = (opcode & 0b00110000) >> 4; 

                    uint16_t reg = get_16bit_reg(CPU_State, identifier); 
                    set_16bit_reg(CPU_State, reg + 1, identifier); 

                    read_byte(mem, 0); // takes an extra m-cycles

                    return;
                }
                case(11): // dec r16
                {
                    uint8_t identifier = (opcode & 0b00110000) >> 4; 

                    uint16_t reg = get_16bit_reg(CPU_State, identifier); 
                    set_16bit_reg(CPU_State, reg - 1, identifier); 
                    
                    read_byte(mem, 0); // takes an extra m-cycles

                    return;
                }
                case(9): // add hl, r16
                {

                    uint8_t identifier = (opcode & 0b00110000) >> 4; 
                    uint16_t old = CPU_State->HL; 
                    uint16_t value = get_16bit_reg(CPU_State, identifier);

                    CPU_State->HL += value; 
                    CPU_State->F &= 0b10011111;
                    
                    if ( ( (old & 0x0FFF) + (value & 0x0FFF)) > 0x0FFF)
                        CPU_State->F |= 0x20; // half carry 
                    if ((uint32_t) old + value > 0xFFFF)
                        CPU_State->F |= 0x10; 
                    CPU_State->F &= 0b10111111; 
                    
                    read_byte(mem, 0); // takes an extra m-cycles

                    return;
                }
            }
        }
    }

    // forbidden opcodes
    if(opcode == 0xD3 || opcode == 0xDB || opcode == 0xDD || opcode == 0xE3 || opcode == 0xE4 || opcode == 0xEB || opcode == 0xEC || opcode == 0xED || opcode == 0xF4 || opcode == 0xFC || opcode == 0xFD)
        CPU_State->state = 'l';
    
}
