#include "gbstructs.h"
#include "ppu.h"

void fifo_fetcher(GBMemory_t *mem){
    // Get tile
    mem->ppu.SCX = (mem->IORegisters[0x43] & 0b11111000) | (mem->ppu.SCX & 0b00000111);

    if(mem->ppu.tile_row >= 8) mem->ppu.tile_row = 0; 


    uint16_t tilemap = 0x9800;

    uint8_t on_screen_x = mem->ppu.screen_x >= 0 && mem->ppu.screen_x < 160; 

    if(!on_screen_x && ((mem->ppu.lcdc >> 3) & 1)) tilemap = 0x9C00; 

    if(on_screen_x && ((mem->ppu.lcdc >> 6) & 1)) tilemap = 0x9C00; 

    uint8_t bg_x = (SCX + screen_x) & 0xFF;
    uint8_t bg_y = (SCY + LY) & 0xFF;   

    uint8_t tile_x = (mem->ppu.fetching_state == 'w') ? bg_x / 8 : ((SCX / 8) + mem->ppu.fetcher_x); 
    uint8_t tile_y =  (mem->ppu.fetching_state == 'w') ? bg_y / 8 : ((mem->IORegisters[0x44] + mem->IORegisters[0x42]) & 225); 
    
    // The fetcher keeps track of which X and Y coordinate of the tile it’s on
    uint8_t pixel_x = bg_x % 8; 
    uint8_t pixel_y = bg_y % 8; 
    
    uint8_t tile_index = mem->VRAM[0][0x8000 - tilemap + (tile_y * 32) + tile_x]; // this is because we index into the 2d array with tile_y and tile_x




    // Get tile data low

    uint8_t tile_low; // stores the lower byte of the tile row we're on

    uint8_t tile_attributes = mem->VRAM[1][0x8000 - tilemap + (tile_y * 32) + tile_x]; 

    uint8_t yflipped = ((tile_attributes >> 6) & 1); // check y flip
    uint8_t hflipped = ((tile_attributes >> 5) & 1); // check h flip
    
    mem->ppu.vram_bank = ((tile_attributes >> 3) & 1); // change ppu's vram bank access

    if((mem->ppu.lcdc >> 4) & 1){
        tile_low = yflipped ? 
                    mem->VRAM[mem->ppu.vram_bank][0x8000 + tile_index * 16 + (7 - row) * 2] 
                    : 
                    mem->VRAM[mem->ppu.vram_bank][0x8000 + tile_index * 16 + row * 2]; 


    }else{
        tile_low = yflipped ? 
                    mem->VRAM[mem->ppu.vram_bank][0x9000 + (int8_t) tile_index * 16 + (7 - row) * 2] 
                    : 
                    mem->VRAM[mem->ppu.vram_bank][0x9000 + (int8_t) tile_index * 16 + row * 2]; 
    }



    // Get tile data high

    uint8_t tile_high; // stores the upper byte of the tile row we're on
    
    if((mem->ppu.lcdc >> 4) & 1){
        tile_high = yflipped ? 
                    mem->VRAM[mem->ppu.vram_bank][0x8000 + tile_index * 16 + (7 - row) * 2 + 1] 
                    : 
                    mem->VRAM[mem->ppu.vram_bank][0x8000 + tile_index * 16 + row * 2 + 1]; 


    }else{
        tile_high = yflipped ? 
                    mem->VRAM[mem->ppu.vram_bank][0x9000 + (int8_t) tile_index * 16 + (7 - row) * 2 + 1] 
                    : 
                    mem->VRAM[mem->ppu.vram_bank][0x9000 + (int8_t) tile_index * 16 + row * 2 + 1]; 
    }



    // Push (only window and bg for rn) 

    if(!mem->ppu.bg_index){ // if it's empty
        if(hflipped){ // put the pixels in the fifo lsb first
            for(int i = 0; i < 8; mem->ppu.bg_index++ ){

                mem->ppu.bg_fifo[mem->ppu.bg_index].color = (((tile_high >> i) & 1) << 1) | ((tile_low >> i) & 1); // look at pandocs to understand
                mem->ppu.bg_fifo[mem->ppu.bg_index].palette = tile_attributes & 7; 
                mem->ppu.bg_fifo[mem->ppu.bg_index].sprite_prio = 0; // irrelavent bc this is bg or widnow object 
                mem->ppu.bg_fifo[mem->ppu.bg_index].bg_prio = (tile_attributes >> 7) & 1;

                i++; 
            }
        } else {
            for(int i = 0; i < 8; mem->ppu.bg_index++ ){

                mem->ppu.bg_fifo[mem->ppu.bg_index].color = (((tile_high >> (7 - i)) & 1) << 1) | ((tile_low >> (7 - i)) & 1); // look at pandocs to understand
                mem->ppu.bg_fifo[mem->ppu.bg_index].palette = tile_attributes & 7; 
                mem->ppu.bg_fifo[mem->ppu.bg_index].sprite_prio = 0; // irrelavent bc this is bg or widnow object 
                mem->ppu.bg_fifo[mem->ppu.bg_index].bg_prio = (tile_attributes >> 7) & 1;

                i++; 
            } 
        }
        
    }
      


    // Sleep
    // do nothin 

    mem->ppu.tile_row++; 

}