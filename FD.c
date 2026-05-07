#include <stdio.h>

void decode(short int instruction) {
        
        short int opcode = 0;
        short int r1 = 0;
        short int r2 = 0;
        int8_t imm = 0;
        int8_t valueR1 = 0;
        int8_t valueR2 = 0;
        
        opcode = (instruction & 0b1111<<12)>>12;
        r1 = (instruction & 0b111111<<6)>>6;
        r2 = instruction & 0b111111;
        imm = instruction & 0b111111;

        if (imm & 0b00100000) 
            imm |= 0b11000000;

        valueR1 = registerFile[r1];
        valueR2 = registerFile[r2];
        
    
        
        
        // Printings
        
		printf("opcode = %i\n",opcode);
		printf("r1 = %i\n",r1);
		printf("r2 = %i\n",r2);
		printf("immediate = %i\n",imm);
		printf("value[r1] = %i\n",valueR1);
		printf("value[r2] = %i\n",valueR2);
		printf("---------- \n");
             
}

void fetch() {
        
        short int instruction = 0;
        
      instruction = instructionMemory[pc];
      printf("Fetched instruction at PC=%d\n", pc);
       pc++;
        
        decode(instruction);
        
       
        
}


