#include <stdio.h>
#include <stdint.h>

#include "cpu_core.h"


int main(int argc, char *argv[]){
    size_t ROMSIZE;
    FILE *RAW_ROM = fopen(argv[1], "rb");

    fseek(RAW_ROM, 0, SEEK_END);
    ROMSIZE = ftell(RAW_ROM);
    rewind(RAW_ROM);

    uint8_t *ROM = ProcessROM(RAW_ROM, ROMSIZE);


    return 0;

}
