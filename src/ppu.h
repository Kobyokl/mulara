#pragma once

#include "bus.h"

typedef struct Pixel {
    uint8_t color; // value between 0 and 3
    uint8_t palette; // value between 0 and 7 
    uint8_t sprite_prio; // the oam index for the object
    uint8_t bg_prio; // has the obj vs bg priority shii
} Pixel_t ; 




// condiders lcdc.4 and the correct tile index of the tile map 
uint8_t bg_color_index(uint8_t bg); 


void mode_switcher(GBMemory_t *mem); 
void set_ppu_mode(GBMemory_t *mem, uint8_t mode, uint16_t dots); 
char bg_obj_priority(GBMemory_t *mem, uint16_t bg, uint16_t obj); 
void stat_handlin(GBMemory_t *mem); 
void ppu_update(GBMemory_t *mem); 