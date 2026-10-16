#include "ppu.h"
#include "interrupt.h"


void stat_handlin(GBMemory_t *mem){
    uint8_t stat = mem->IORegisters[0x41];
    uint8_t ly_eq_lyc = mem->IORegisters[0x44] == mem->IORegisters[0x45];

    uint8_t stat_line = // logical oring & anding assures things stay within 1 bit rather than bytewise oring and we dont have to do any bitshifting
        ((stat & 0x08) && mem->ppu.mode == 0) ||
        ((stat & 0x10) && mem->ppu.mode == 1) ||
        ((stat & 0x20) && mem->ppu.mode == 2) ||
        ((stat & 0x40) && ly_eq_lyc);

    mem->IORegisters[0x41] &= 0xF8;
    mem->IORegisters[0x41] |= (ly_eq_lyc << 2); // ly == lyc flag 
    mem->IORegisters[0x41] |= (mem->ppu.mode & 0x03); // updates bottom 2 bits of stat reg


    if (stat_line && !mem->ppu.stat_or) {
        request_interrupt(LCD, mem);
    }
    

    mem->ppu.stat_or = stat_line;
}


// did chatgpt write the following 2 functions? yes. do i care? no

void set_ppu_mode(GBMemory_t *mem, uint8_t mode, uint16_t dots){
    mem->ppu.mode = mode;
    mem->ppu.rem_dots = dots;
}

void mode_switcher(GBMemory_t *mem) {
    if(mem->ppu.rem_dots != 0) return;

    switch(mem->ppu.mode){

        case 2: // mode 2  -> mode 3
        {
            mem->ppu.line_penalty = penalty_handler(mem);
            mem->ppu.screen_x = 0;

            set_ppu_mode(
                mem,
                3,
                172 + mem->ppu.line_penalty
            );
            

            break;
        }
        case 3: //mode 3  -> mode 0 
            set_ppu_mode(
                mem,
                0,
                204 - mem->ppu.line_penalty
            );

            break;

        case 0:
            if(mem->IORegisters[0x44] < 144){
                set_ppu_mode(mem, 2, 80);
            }
            else{
                set_ppu_mode(mem, 1, 4560);
                request_interrupt(VBlank, mem);
            }

            break;

        case 1:
            mem->IORegisters[0x44] = 0;
            set_ppu_mode(mem, 2, 80);
            break;
    }
}

uint8_t penalty_handler(GBMemory_t *mem){
    uint8_t penalty = 0; 

    penalty += mem->IORegisters[0x43] % 8;

    if(mem->ppu.screen_x + 6 == mem->IORegisters[0x4B] && mem->ppu.y_condition && (mem->ppu.lcdc >> 5)){
        // After the last non-window pixel is emitted, a 6-dot penalty is incurred
        penalty += 6; 
    }

    return penalty; 

}

char bg_obj_priority(GBMemory_t *mem, uint16_t bg, uint16_t obj){ 
    // takes addr of obj and the index of bg (index that appears in the tile map)
    // 'o' = object 
    // 'b' = background

    uint8_t bg_map_attr; // fill this out later
    uint8_t OAM_attr; // fill this out later

    uint8_t prio_key =  ((mem->ppu.lcdc & 1) << 2)  // bit 0 of lcdc
                        | (((OAM_attr >> 7 ) & 1) << 1)      // bg map attribute bit 7
                        | (bg_map_attr >> 7) & 1 ;                // OAM attribute bit 7

    switch(prio_key){
        case 0: return 'o';
        case 1: return 'o'; 
        case 2: return 'o'; 
        case 3: return 'o'; 
        case 4: return 'o'; 
        case 5: return bg_color_index(bg) > 0 ? 'b' : 'o';
        case 6: return bg_color_index(bg) > 0 ? 'b' : 'o';
        case 7: return bg_color_index(bg) > 0 ? 'b' : 'o';
    }
}


void ppu_update(GBMemory_t *mem){
    mem->ppu.active = (mem->ppu.lcdc >> 7) & 1; 
    if(!mem->ppu.active) return; 

    // handle the actions per mode
    switch(mem->ppu.mode){
        case 2: break; // search of objs that overlap the scanline we're on 


        case 3:  // send pixels to the lcd 
        {

            // fetch pixel to add to buffer
            // pixel_fetch(mem); or something


            if(mem->ppu.screen_x + 7 == mem->IORegisters[0x4B] && mem->ppu.y_condition && (mem->ppu.lcdc >> 5)){ // if we're on a pixel where were goign to render the window
                mem->ppu.fetching_state = 'w';
            } 
            

             
             //if we actually fukin render a pixel thennn: 
                mem->ppu.screen_x++; 
            
            break;
        } 



        case 0: break; // wait until the end of the scanline 
        case 1: break; // wait until the next frame 
    }

    mem->ppu.rem_dots--; 


    if (mem->ppu.dot >= 455) {
        mem->IORegisters[0x44]++; // increment the scanline
        mem->ppu.SCX = mem->IORegisters[0x43]; 
        mem->ppu.SCY = mem->IORegisters[0x42]; 

        if(mem->IORegisters[0x4A] == mem->IORegisters[0x44]{ // check the LY condition
            mem->ppu.y_condition = 1; 
        } 

        mem->ppu.dot = 0; 
        
    }else{ mem->ppu.dot++; }

    mode_switcher(mem);
    stat_handlin(mem);
}









