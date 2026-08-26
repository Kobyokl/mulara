#include "ppu.h"

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