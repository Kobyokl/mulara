#include "cpu_core.h"

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

uint16_t set_16bitstk(SharpState_t *CPU, uint8_t identifier, uint16_t value){
    switch(identifier){
        case(0): CPU->BC = value; return CPU->BC;
        case(1): CPU->DE = value; return CPU->DE;
        case(2): CPU->HL = value; return CPU->HL;
        case(3):
        {
            CPU->A = (uint8_t) ((value & 0xFF00) >> 8);
            CPU->F = (uint8_t) (value & 0x00FF);

            return (((uint16_t) CPU->A) << 8 ) | CPU->F;
        }
    }
}

uint16_t get_16bitstk(SharpState_t *CPU, uint8_t identifier){
    switch(identifier){
        case(0): return CPU->BC;
        case(1): return CPU->DE;
        case(2): return CPU->HL;
        case(3): return (((uint16_t) CPU->A) << 8 ) | CPU->F;
    }
}



void instruction_step(SharpState_t *CPU_State, GBMemory_t *mem){
    uint8_t opcode = fetch_byte(mem, CPU_State);
    uint8_t block = block_finder(opcode);

    CPU_State->F &= 0xF0;

    switch(opcode){
        case(0): return; // NOP
        case(16): // stop (2 bytes)
        {
            CPU_State->state = 's';
            mem->IORegisters[0x04] = 0; 
            fetch_byte(mem, CPU_State);
            return;
        }
        case(0b01110110): // halt
        {
            CPU_State->state = 'h';
            return;
        }
        case(0b00011000): // jr imm8
        {
            //JR n16 takes 3 m-cycles
            CPU_State->PC += (int8_t) fetch_byte(mem, CPU_State);
            read_byte(mem, 0);
            return;
        }
        /*
            Rotate, shift, and bit operation instructions
        */
        case(0b00000111): // rlca
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
        case(0b00001111): // rrca
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
        case(0b00010111): // rla
        {
            uint8_t carry = (CPU_State->A & 0x80) >> 7;
            CPU_State->A = (CPU_State->A << 1) | ((CPU_State->F & 0x10) >> 4); // just like 0x07 but bit zero is dependent on the carry flag BEFORE the flag is modified...
            CPU_State->F = carry << 4; // only carry flag
            return;
        }
        case(0b00011111): // rra
        {
            uint8_t carry = (CPU_State->A & 0x01);
            CPU_State->A = (CPU_State->A >> 1) | (CPU_State->F & 0x10); // just like 0x07 but bit zero is dependent on the carry flag BEFORE the flag is modified...
            CPU_State->F = carry << 4; // only carry flag
            return;
        }
        case(0b00100111): // daa
        {
            // this was taken from someone else's emulator icl

            uint32_t temp = (((uint32_t)CPU_State->A) << (3 + 1))
                        | ((((uint32_t)CPU_State->F >> 4) & 7) << 1);
            CPU_State->A = gb_daa_table[temp];
            CPU_State->F = gb_daa_table[temp + 1];
            return;
        }
        case(0b00101111): // cpl
        {
            CPU_State->A = ~CPU_State->A;
            CPU_State->F |= 0b01100000;
            return;
        }
        case(0b00110111): // scf
        {
            CPU_State->F |= 0b00010000;
            CPU_State->F &= 0b10011111;
            return;
        }
        case(0b00111111): // ccf
        {
            CPU_State->F ^= 0b00010000;
            CPU_State->F &= 0b10011111;
            return;
        }
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
                    int8_t offset = (int8_t) fetch_byte(mem, CPU_State);
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
        case(1): // block 1 (ld r8, r8)
        {
            uint8_t dest = (opcode & 0b00111000) >> 3;
            uint8_t source = (opcode & 0b00000111);
            if(dest == source) return; // just cuz

            uint8_t val = get_8bit_reg(mem, CPU_State, source);

            set_8bit_reg(mem, CPU_State, val, dest);
        }
        case(2): // block 2 (8 bit arithmatic)
        {
            uint8_t source = (opcode & 0b00000111);

            switch((uint8_t) (opcode >> 3)){

                case(16): // add
                {
                    uint8_t addVal = get_8bit_reg(mem, CPU_State, source);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint16_t result = A + addVal;

                    CPU_State->A = (uint8_t) result;

                    CPU_State->F = 0x00;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) + (addVal & 0x0F) > 0x0F) CPU_State->F |= 0x20;
                    if (result > 0xFF) CPU_State->F |= 0x10;
                    break;

                }
                case(17): // adc
                {
                    uint8_t addVal = get_8bit_reg(mem, CPU_State, source);
                    uint8_t carryVal = (CPU_State->F & 0b00010000) >> 4;
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint16_t result = A + addVal + carryVal;

                    CPU_State->A = (uint8_t) result;

                    CPU_State->F = 0x00;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) + (addVal & 0x0F) > 0x0F) CPU_State->F |= 0x20;
                    if (result > 0xFF) CPU_State->F |= 0x10;
                    break;
                }
                case(18): //sub a, r8
                {
                    uint8_t subVal = get_8bit_reg(mem, CPU_State, source);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint8_t result = A - subVal;

                    CPU_State->A = result;

                    CPU_State->F = 0x40;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) < (subVal & 0x0F)) CPU_State->F |= 0x20;
                    if (A < subVal) CPU_State->F |= 0x10;
                    break;
                }
                case(19):
                {
                    uint8_t subVal = get_8bit_reg(mem, CPU_State, source);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint8_t carryVal = (CPU_State->F & 0b00010000) >> 4;
                    int8_t result = A - subVal - carryVal;

                    CPU_State->A = result;

                    CPU_State->F = 0x40;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) < (subVal & 0x0F)) CPU_State->F |= 0x20;
                    if (A < subVal) CPU_State->F |= 0x10;
                    break;
                }
                case(20):
                {
                    uint8_t andVal = get_8bit_reg(mem, CPU_State, source);

                    CPU_State->A &= andVal;

                    CPU_State->F = 0x20;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }
                case(21):
                {
                    uint8_t xorVal = get_8bit_reg(mem, CPU_State, source);

                    CPU_State->A ^= xorVal;

                    CPU_State->F = 0x00;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }
                case(22):
                {
                    uint8_t orVal = get_8bit_reg(mem, CPU_State, source);

                    CPU_State->A |= orVal;

                    CPU_State->F = 0x00;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }
                case(23): // cp a, r8 ... same as sub but doesnt update value of accumulator
                {
                    uint8_t subVal = get_8bit_reg(mem, CPU_State, source);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint8_t result = A - subVal;

                    CPU_State->F = 0x40;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) < (subVal & 0x0F)) CPU_State->F |= 0x20;
                    if (A < subVal) CPU_State->F |= 0x10;
                    break;
                }
            }
        }
        case(3): // block 3
        {
            switch(opcode){
                case(0b11000110):
                {
                    uint8_t addVal = fetch_byte(mem, CPU_State);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint16_t result = A + addVal;

                    CPU_State->A = (uint8_t) result;

                    CPU_State->F = 0x00;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) + (addVal & 0x0F) > 0x0F) CPU_State->F |= 0x20;
                    if (result > 0xFF) CPU_State->F |= 0x10;
                    break;
                }
                case(0b11001110):
                {
                    uint8_t addVal = fetch_byte(mem, CPU_State);
                    uint8_t carryVal = (CPU_State->F & 0b00010000) >> 4;
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint16_t result = A + addVal + carryVal;

                    CPU_State->A = (uint8_t) result;

                    CPU_State->F = 0x00;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) + (addVal & 0x0F) > 0x0F) CPU_State->F |= 0x20;
                    if (result > 0xFF) CPU_State->F |= 0x10;
                    break;
                }
                case(0b11010110):
                {
                    uint8_t subVal = fetch_byte(mem, CPU_State);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint8_t result = A - subVal;

                    CPU_State->A = result;

                    CPU_State->F = 0x40;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) < (subVal & 0x0F)) CPU_State->F |= 0x20;
                    if (A < subVal) CPU_State->F |= 0x10;
                    break;
                }
                case(0b11011110):
                {
                    uint8_t subVal = fetch_byte(mem, CPU_State);
                    uint8_t A = CPU_State->A; // need this for half carry logic
                    uint8_t carryVal = (CPU_State->F & 0b00010000) >> 4;
                    int8_t result = A - subVal - carryVal;

                    CPU_State->A = result;

                    CPU_State->F = 0x40;

                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    if ((A & 0x0F) < (subVal & 0x0F)) CPU_State->F |= 0x20;
                    if (A < subVal) CPU_State->F |= 0x10;
                    break;
                }
                case(0b11100110):
                {
                    uint8_t andVal = fetch_byte(mem, CPU_State);

                    CPU_State->A &= andVal;

                    CPU_State->F = 0x20;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }
                case(0b11110110):
                {
                    uint8_t xorVal = fetch_byte(mem, CPU_State);

                    CPU_State->A ^= xorVal;

                    CPU_State->F = 0x00;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }
                case(0b11111110):
                {
                    uint8_t orVal = fetch_byte(mem, CPU_State);

                    CPU_State->A |= orVal;

                    CPU_State->F = 0x00;
                    if (CPU_State->A == 0) CPU_State->F |= 0x80;
                    break;
                }

                /* =====================================================================
                    Above was literally just the same as block 2 but with immediates
                   =====================================================================
                */
               case(0b11001001):
               {
                    uint8_t spLSB = read_byte(mem, CPU_State->SP++); // reads whatever value's at the stack pointer the increments the stack pointer
                    uint8_t spMSB = read_byte(mem, CPU_State->SP++);

                    CPU_State->PC = (spMSB << 8) | spLSB;

                    read_byte(mem, 0); // take an extra cycle cuz the instruction takes 4 m-cycles
                    break;
               }
               case(0b11011001):
               {
                    uint8_t spLSB = read_byte(mem, CPU_State->SP++); // reads whatever value's at the stack pointer the increments the stack pointer
                    uint8_t spMSB = read_byte(mem, CPU_State->SP++);

                    CPU_State->PC = (spMSB << 8) | spLSB;
                    CPU_State->IME = 1;

                    read_byte(mem, 0); // take an extra cycle cuz the instruction takes 4 m-cycles
                    break;
               }
               case(0b11000011): /// jp imm16
               {
                    uint8_t LSB = fetch_byte(mem, CPU_State);
                    uint8_t MSB = fetch_byte(mem, CPU_State);

                    CPU_State->PC = (MSB << 8) | LSB;

                    read_byte(mem, 0); // take an extra cycle cuz the instruction takes 4 m-cycles
                    break;
               }
               case(0b11101001): // jp hl
               {
                    CPU_State->PC = CPU_State->HL;
                    break;
               }
               case(0b11001101): // call imm16
               {
                    uint8_t LSB = fetch_byte(mem, CPU_State); // M2
                    uint8_t MSB = fetch_byte(mem, CPU_State); // M3
                    uint16_t addr = (MSB << 8) | LSB;

                    read_byte(mem, 0);

                    write_byte(mem, --CPU_State->SP, CPU_State->PC & 0xFF);

                    write_byte(mem, --CPU_State->SP, (CPU_State->PC >> 8) & 0xFF);

                    CPU_State->PC = addr;
                    break;
               }
               case(0b11100010): // ldh [c], a
               {
                    uint8_t C = (uint8_t) (CPU_State->BC & 0x00FF);

                    write_byte(mem, (uint16_t) (0xFF00 | C), CPU_State->A);

                    break;
               }
               case(0b11100000):
               {
                    uint8_t low_address = fetch_byte(mem, CPU_State);

                    write_byte(mem, (uint16_t) (0xFF00 | low_address), CPU_State->A);
                    break;
               }
               case(0b11101010):
               {
                    uint8_t LSB = fetch_byte(mem, CPU_State); // M2
                    uint8_t MSB = fetch_byte(mem, CPU_State); // M3
                    uint16_t addr = (MSB << 8) | LSB;

                    write_byte(mem, addr, CPU_State->A);
                    break;
               }
               case(0b11110010):
               {
                    uint8_t C = (uint8_t) (CPU_State->BC & 0x00FF);
                    uint8_t value = read_byte(mem, (uint16_t) (0xFF00 | C));

                    CPU_State->A = value;
                    break;
               }
               case(0b11110000):
               {
                    uint8_t low_address = fetch_byte(mem, CPU_State);
                    uint8_t value = read_byte(mem, (uint16_t) (0xFF00 | low_address));

                    CPU_State->A = value;
                    break;

               }
               case(0b11111010): // ld a, [imm16]
               {
                    uint8_t LSB = fetch_byte(mem, CPU_State); // M2
                    uint8_t MSB = fetch_byte(mem, CPU_State); // M3
                    uint8_t value = read_byte(mem, (uint16_t) (MSB << 8) | LSB);

                    CPU_State->A = value;
                    break;
               }
               case(0b11101000): // get cycles right here
               {
                    int8_t addVal = fetch_byte(mem, CPU_State);
                    uint16_t SP = CPU_State->SP;

                    CPU_State->SP += addVal;

                    CPU_State->F = 0x00;
                    if (((SP & 0xF) + ((uint16_t)addVal & 0xF)) > 0xF) CPU_State->F |= 0x20; // H
                    if (((SP & 0xFF) + ((uint16_t)addVal & 0xFF)) > 0xFF) CPU_State->F |= 0x10; // C

                    read_byte(mem, 0); // take extra cycles
                    read_byte(mem, 0);
                    break;

               }
               case(0b11111000): // get cycles right here as well
               {
                    int8_t addVal = fetch_byte(mem, CPU_State);
                    uint16_t SP = CPU_State->SP;

                    CPU_State->F = 0x00;
                    if (((SP & 0xF) + ((uint16_t)addVal & 0xF)) > 0xF) CPU_State->F |= 0x20; // H
                    if (((SP & 0xFF) + ((uint16_t)addVal & 0xFF)) > 0xFF) CPU_State->F |= 0x10; // C

                    CPU_State->HL = SP + addVal;

                    read_byte(mem, 0); // take extra cycle
                    break;
               }
               case(0b11111001):
               {
                    CPU_State->SP = CPU_State->HL;

                    read_byte(mem, 0); // take extra cycle
                    break;
               }
               case(0b11110011):
               {
                    CPU_State->IME = 0;
                    break;
               }
               case(0b11111011):
               {
                    CPU_State->IME_next = 1;
                    break;
               }
               case(0b11001011): // CB Prefix
               {
                    uint8_t next_opcode = fetch_byte(mem, CPU_State);

                    switch(next_opcode){
                        uint8_t cbBlock = block_finder(next_opcode);
                        uint8_t identifier = next_opcode & 0b00000111;
                        uint8_t bit_index = (next_opcode & 0b00111000) >> 3;

                        case(0):
                        {
                            uint8_t instruction_identifier = (next_opcode & 0b11111000) >> 3;

                            switch(instruction_identifier){
                                case(0): /// rlc r8
                                {
                                    CPU_State->F &= 0; //unset all other flags

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);

                                    if(_register & 0x80){
                                        CPU_State->F |= 0b00010000; // set the carry flag
                                        set_8bit_reg(mem, CPU_State, ((_register << 1) + 1), identifier);
                                    }else{
                                        set_8bit_reg(mem, CPU_State, _register << 1, identifier);;
                                    }

                                    if(~get_8bit_reg(mem, CPU_State, identifier)) CPU_State->F |= 0x80;

                                    return; // usign returns here cuz idk if break is goonna break out of all these switch cases...
                                }
                                case(1):
                                {
                                    CPU_State->F &= 0; //unset all other flags

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                                    uint8_t updated_register;
                                    if(_register & 1){
                                        CPU_State->F |= 0b00010000; // set the carry flag
                                        updated_register = set_8bit_reg(mem, CPU_State, ((_register >> 1) | 0x80), identifier);
                                    }else{
                                        updated_register = set_8bit_reg(mem, CPU_State, _register >> 1, identifier);;
                                    }

                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero

                                    return;
                                }
                                case(2):
                                {
                                    CPU_State->F &= 0;

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                                    uint8_t carry_bit = (CPU_State->F & 0x10) >> 4;

                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, (_register << 1) + carry_bit , identifier);

                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero
                                    if(_register & 0b10000000) CPU_State->F |= 0x10;
                                    return;
                                }
                                case(3):
                                {
                                    CPU_State->F &= 0;

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                                    uint8_t carry_bit = (CPU_State->F & 0x10) << 3;

                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, (_register << 1) | carry_bit , identifier);


                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero
                                    if(_register & 0b00000001) CPU_State->F |= 0x10;

                                    return;
                                }
                                case(4):
                                {
                                    CPU_State->F &= 0;

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);

                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, (_register << 1) , identifier);

                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero
                                    if(_register & 0b10000000) CPU_State->F |= 0x10; // carry bit
                                    return;
                                }
                                case(5):
                                {
                                    CPU_State->F &= 0;

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                                    uint8_t shift_arith = (uint8_t) (((int8_t) _register) >> 1);
                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, shift_arith , identifier);

                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero
                                    if(_register & 0b00000001) CPU_State->F |= 0x10; // carry bit
                                    return;
                                }
                                case(6):
                                {
                                    CPU_State->F = 0x80;

                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);

                                    // swap the 2 nibbles in the register
                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, ((_register & 0b00001111) << 4) | ((_register & 0b11110000) >> 4), identifier);
                                    return;
                                }
                                case(7):
                                {
                                    uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                                    uint8_t shift_arith = _register >> 1;
                                    uint8_t updated_register = set_8bit_reg(mem, CPU_State, shift_arith , identifier);

                                    if(!updated_register) CPU_State->F |= 0x80; // if its zero
                                    if(_register & 0b10000000) CPU_State->F |= 0x10; // carry bit
                                    return;
                                }
                            }
                        }
                        case(1):
                        {
                            uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);

                            if(!(_register >> bit_index) & 1) CPU_State->F |= 0x80;

                            CPU_State->F &= 0b10110000;
                            CPU_State->F |= 0x20;
                            break;
                        }
                        case(2):
                        {
                            uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                            uint8_t value = _register & (~(1 << bit_index));

                            set_8bit_reg(mem, CPU_State, value, identifier);

                            return;
                        }
                        case(3):
                        {
                            uint8_t _register = get_8bit_reg(mem, CPU_State, identifier);
                            uint8_t value = _register | (1 << bit_index);

                            set_8bit_reg(mem, CPU_State, value, identifier);

                            return;
                        }
                    }
               }
               switch(opcode & 0b00000111){
                    case(0):
                    {
                        uint8_t condition = get_condition(CPU_State, (opcode & 0b00011000) >> 3);
                        uint8_t spLSB, spMSB;
                        if(condition){
                            spLSB = read_byte(mem, CPU_State->SP++); // reads whatever value's at the stack pointer the increments the stack pointer
                            spMSB = read_byte(mem, CPU_State->SP++);
                            CPU_State->PC = (spMSB << 8) | spLSB;

                            read_byte(mem, 0); // take an extra cycle cuz the instruction takes extra m-cycle
                            read_byte(mem, 0); // take an extra cycle cuz the instruction takes extra m-cycle
                        }

                        read_byte(mem, 0); // take an extra cycle cuz the instruction takes extra m-cycle
                        return;
                    }
                    case(2):
                    {
                        uint8_t condition = get_condition(CPU_State, (opcode & 0b00011000) >> 3);
                        uint8_t LSB = fetch_byte(mem, CPU_State);;
                        uint8_t MSB = fetch_byte(mem, CPU_State);;

                        if(condition)
                            CPU_State->PC = (MSB << 8) | LSB;
                            read_byte(mem, 0); // take an extra cycle cuz the instruction takes extra m-cycle

                        return;
                    }
                    case(4):
                    {
                        uint8_t condition = get_condition(CPU_State, (opcode & 0b00011000) >> 3);
                        uint8_t LSB = fetch_byte(mem, CPU_State); // M2
                        uint8_t MSB = fetch_byte(mem, CPU_State); // M3

                        uint16_t addr = (MSB << 8) | LSB;

                        read_byte(mem, 0);

                        if(condition){
                            write_byte(mem, --CPU_State->SP, CPU_State->PC & 0xFF);
                            write_byte(mem, --CPU_State->SP, (CPU_State->PC >> 8) & 0xFF);

                            read_byte(mem ,0);

                            CPU_State->PC = addr;
                        }

                        return;
                    }
                    case(7):
                    {
                        uint16_t addr = (opcode & 0b00111000); // ight shift by 3 then left shift by 3 cuz have to multiply by 8


                        write_byte(mem, --CPU_State->SP, CPU_State->PC & 0xFF);
                        write_byte(mem, --CPU_State->SP, (CPU_State->PC >> 8) & 0xFF);

                        read_byte(mem, 0);

                        CPU_State->PC = addr;
                        return;
                    }
                    case(1):
                    {
                        uint8_t identifier = (opcode & 0b00110000) >> 4;

                        uint8_t lsb = read_byte(mem, CPU_State->SP++);
                        uint8_t msb = read_byte(mem, CPU_State->SP++);
                        uint16_t value = (uint16_t) (msb << 8) | lsb;

                        set_16bitstk(CPU_State, identifier, value);
                        return;
                    }
                    case(5):
                    {
                        uint8_t identifier = (opcode & 0b00110000) >> 4;
                        uint16_t _register = get_16bitstk(CPU_State, identifier);

                        write_byte(mem, --CPU_State->SP, _register & 0xFF);
                        write_byte(mem, --CPU_State->SP, (_register >> 8) & 0xFF);

                        read_byte(mem, 0);
                        return;
                    }
               }
            }
        }
    }

    // forbidden opcodes
    if(opcode == 0xD3 || opcode == 0xDB || opcode == 0xDD || opcode == 0xE3 || opcode == 0xE4 || opcode == 0xEB || opcode == 0xEC || opcode == 0xED || opcode == 0xF4 || opcode == 0xFC || opcode == 0xFD)
        CPU_State->state = 'l'; // locked

}
