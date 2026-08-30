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

void ppu_update(GBMemory_t *mem){
    // we will handle the status registers 
    // on each call of this function (each dot) we also we either otuput a pixel or we're stalling the FIFOs
    
    // handle lcd interrupt calling (aka STAT interrupt)
    uint8_t lyc_select = (mem->IORegisters[0x41] >> 6) & 1; 
    if(lyc_select){
        if(mem->IORegisters[0x45] == mem->IORegisters[0x44]){
            uint8_t ly_lyc = ((mem->IORegisters[0x41] >> 2) & 1); 
            
            if(!ly_lyc) request_interrupt(LCD, mem); 
            
            mem->IORegisters[0x41] |= 4; // set the lyc == ly bit
        }else{
            mem->IORegisters[0x41] &= ~((uint8_t) 4);
        }
    }else{
        uint8_t mode_select = (mem->IORegisters[0x41] >> 3 ) & 7; 
        
        switch(mode_select){
            case(1):
            {
                // selected mode 0 for stat interrupt
                if(mem->ppu.dot
            }
            case(2):
            {
                // selected mode 1 for 
            }
            case(4):
            {
                // selected mode 2 for stat interrupt 
            }
        }
    }


    if(mem->ppu.mode == 2 && mem->ppu.rem_dots <= 0){
        mem->ppu.mode = 3;
        mem->ppu.rem_dots = 172; 
    }else if(mem->ppu.mode == 3 && mem->ppu.rem_dots <= 0 ){
        mem->ppu.mode = 0;
        mem->ppu.rem_dots = 87; 
    }else if(mem->ppu.mode == 0 && mem->ppu.rem_dots <= 0 && !(mem->IORegisters[0x44] == 145)){
        mem->ppu.mode = 2;
        mem->ppu.rem_dots = 80; 
    }else if(mem->ppu.mode == 0 && mem->ppu.rem_dots <= 0 && !(mem->IORegisters[0x44] == 145)){
        mem->ppu.mode = 1; 
        mem->ppu.rem_dots = 4560; 
    }else{
        mem->ppu.rem_dots--; 
    }
    

    mem->ppu.dot = (mem->ppu.dot == 455) ? 0 : ++mem->ppu.dot; 
    


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