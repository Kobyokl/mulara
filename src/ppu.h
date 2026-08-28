#include "bus.h"

typedef struct PPU {

    unsigned int dot;  
    unsigned int line_counter; 
    unsigned int window_line_counter; 
    // idrk what to put here yet

} PPU_t;

typedef struct Tile {

    // for these arrays bit 7 is the leftmost pixel adn bit 0 is the rightmost pixel
    // this is for both bytes and they come together to create the 2 bit color_depth for each pixxel
    // so the bits of the two arrays will be combined to create the tile mapping
    

    // if tile data is  tile data $3C $7E $42 $42 $42 $42 $42 $42 $7E $5E $7E $0A $7C $56 $38 $7C
    // then these array contents will be as follows: 

    uint8_t a_h[8]; // 3C 42 42 42 7E 7E 7C 38
    uint8_t i_p[8]; // 7E 42 42 42 5E 0A 56 7C

    // ^^^ these arrays may not be stored in this struct in the future...

    uint8_t color_depths[8][8]; // will store the tile's color indexes in a 2D array by combining a_h and i_p..  
    
    // the combination of the two 1D arrays will look something like this :
    
    /*
            for(int i = 0; i < 8; i++){
                for(int j = 0; j < 8; j++){
                    color_depths[i][j] = 
                    ((i_p[i] >> (7 - j) & 1) << 1) 
                    |
                    ((a_h[i] >> (7 - j)) & 1); 
                }
            }
    */ 
} Tile_t;

// condiders lcdc.4 and the correct tile index of the tile map 
uint8_t bg_color_index(uint8_t bg); 