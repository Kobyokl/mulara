#include "ppu.h"
#include "interrupt.h"


// PPU Struct definition
 
// typedef struct PPU {

//     unsigned int dot;  
//     unsigned int line_counter; 
//     unsigned int window_line_counter; 
//     // idrk what to put here yet

//     unsigned char mode; // 0, 1, 2, or 3

//     unsigned char rem_penalty; //handles mode 3 dot penalties

// } PPU_t;
void stat_handlin(GBMemory_t *mem){
    uint8_t stat = mem->IORegisters[0x41];
    uint8_t ly_eq_lyc = mem->IORegisters[0x44] == mem->IORegisters[0x45];

    uint8_t stat_line = // logical oring assures things stay within 1 bit rather than bytewise oring and we dont have to do any bitshifting
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

void mode_switcher(GBMemory_t *mem){
    // mode switcher
    if(mem->ppu.mode == 2 && mem->ppu.rem_dots <= 0){
        mem->ppu.mode = 3;
        mem->ppu.rem_dots = 172;

    }else if(mem->ppu.mode == 3 && mem->ppu.rem_dots <= 0 ){
        mem->ppu.mode = 0;        
        mem->ppu.rem_dots = 204; 
    
    }else if(mem->ppu.mode == 0 && mem->ppu.rem_dots <= 0 && mem->IORegisters[0x44] < 144 ){
        mem->ppu.mode = 2;
        mem->ppu.rem_dots = 80; 

    }else if(mem->ppu.mode == 0 && mem->ppu.rem_dots <= 0 && mem->IORegisters[0x44] >= 144){
        mem->ppu.mode = 1; 
        mem->ppu.rem_dots = 4560; 

    }else if(mem->ppu.mode == 1 && mem->ppu.rem_dots <= 0){ // we're done vblank and we can move to processing next framm

        mem->ppu.mode = 2; 
        mem->ppu.rem_dots = 80;

        mem->IORegisters[0x44] = 0; // restart LY register to start processing next frame
    }

}

uint8_t penalty_handler(GBMemory_t *mem){
    return 0; 
}

void ppu_update(GBMemory_t *mem){
    mem->ppu.active = (mem->IORegisters[0x40] >> 7) & 1; 
    if(!mem->ppu.active) return; 

    // handle the actions per mode
    switch(mem->ppu.mode){
        case 2: break; // search fo objs that overlap the scanline we're on 
        case 3: break; // send pixels to the lcd 
        case 0: break; // wait until the end of the scanline 
        case 1: break; // wait until the next frame 
    }

    mem->ppu.rem_dots--; 


    if (mem->ppu.dot >= 455) {
        mem->IORegisters[0x44]++; 
        mem->ppu.dot = 0; 
    }else{ mem->ppu.dot++; }

    mode_switcher(mem);
    stat_handlin(mem);
}









char bg_obj_priority(GBMemory_t *mem, uint16_t bg, uint16_t obj){ 
    // takes addr of obj and the index of bg (index that appears in the tile map)
    // 'o' = object 
    // 'b' = background

    uint8_t bg_map_attr; // fill this out later
    uint8_t OAM_attr; // fill this out later

    uint8_t prio_key =  ((mem->IORegisters[0x40] & 1) << 2)  // bit 0 of lcdc
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