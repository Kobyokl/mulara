//prolly finna delete or move this file somewhere else idk.. 

#include <stdio.h>
#include "gbstructs.h" 

uint8_t* ProcessROM(FILE *ROM, size_t ROMSIZE){
    /*
        processes a ROM file and turns it into an array of uint8's
    */
    if(ROM == NULL){
        printf("\nROM not loaded\n");
        return NULL;
    }

    // Read file byte by byte
    uint8_t *ROMBuffer = malloc(ROMSIZE);

    fread(ROMBuffer, ROMSIZE, 1, ROM);
    fclose(ROM);

    return ROMBuffer;
}