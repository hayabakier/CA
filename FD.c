#include <stdio.h>
#include <stdint.h>
#include "memory.h"
#include "parser.h"
#include "memory.c"
#include "parser.c"
#include "execute.c"
#include "registers.h"

void decode(short int instruction,short int instructionPC) {
        
        int8_t opcode = 0;
        int8_t r1 = 0;
        int8_t r2 = 0;
        int8_t imm = 0;
        int8_t valueR1 = 0;
        int8_t valueR2 = 0;
        
        opcode = ((instruction >> 12) & 0b1111);
        r1 = (instruction >> 6) & 0b111111;
        r2 = instruction & 0b111111;
        imm = instruction & 0b111111;

        if (imm & 0b100000) 
            imm |= 0b11000000;

        valueR1 = registerFile[r1];
        valueR2 = registerFile[r2];
        
    
        
        
        // Printings
        
    printf("opcode = %i\n", opcode);
    printf("r1 = %i\n", r1);

    // R-format
    if (opcode == 0 || opcode == 1 || opcode == 2 || opcode == 5 || opcode == 6 || opcode == 7) {
        printf("r2 = %i\n", r2);
        printf("value[r1] = %i\n", valueR1);
        printf("value[r2] = %i\n", valueR2);
    } 
    // I-format
    else {
        printf("immediate = %i\n", imm);
        printf("value[r1] = %i\n", valueR1);
    }

    printf("---------- \n");
    
    execute(instruction, instructionPC);
}

void fetch() {
    short int instructionPC = pc;
    short int instruction = instructionMemory[pc];
     pc++;
    decode(instruction, instructionPC);
}


