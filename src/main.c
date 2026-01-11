#include <stdio.h>
#include <stdint.h>
#include <gbstructs.h>

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


int main(int argc, char *argv[]){
    size_t ROMSIZE;
    FILE *RAW_ROM = fopen(argv[1], "rb");
    uint8_t *ROM;

    fseek(RAW_ROM, 0, SEEK_END);
    ROMSIZE = ftell(RAW_ROM);
    rewind(RAW_ROM);

    ROM = ProcessROM(RAW_ROM, ROMSIZE);


    return 0;

}
