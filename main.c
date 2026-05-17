#include <stdio.h>
#include "memory.h"
#include "parser.h"
#include "FD.c"

int main() {
    parseFile("program.txt");
    int cycle = 1;
    while (pc < NumberofInstructions) {
    printf("Clock Cycle %d\n", cycle++);
    fetch();
    }
    printf("\nFinal Registers:\n");
    for (int i = 0; i < 64; i++)
        printf("R%d = %d\n", i, registerFile[i]);
    printf("PC = %u\n", pc);
    printf("SREG = %u\n", sreg);

    printf("\nData Memory:\n");
    for (int i = 0; i < 2048; i++)
        if (dataMemory[i] != 0)
            printf("dataMemory[%d] = %d\n", i, dataMemory[i]);

    return 0;
}