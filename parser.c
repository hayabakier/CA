#include "parser.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "memory.h"

short int encodeRType(int opcode, int r1, int r2) {

    short int instruction = 0;

    instruction |= (opcode << 12);
    instruction |= (r1 << 6);
    instruction |= r2;

    return instruction;
}

short int encodeIType(int opcode, int r1, int imm) {

    short int instruction = 0;

    imm &= 0b111111;

    instruction |= (opcode << 12);
    instruction |= (r1 << 6);
    instruction |= imm;

    return instruction;
}
void parseFile(char *filename) {

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[100];

    int instructionIndex = 0;

    while (fgets(line, sizeof(line), file)) {

        char *instruction = strtok(line, " \n");
        char *operand1 = strtok(NULL, " \n");
        char *operand2 = strtok(NULL, " \n");

        short int encodedInstruction = 0;

        if (strcmp(instruction, "ADD") == 0) {

            int r1 = atoi(operand1 + 1);
            int r2 = atoi(operand2 + 1);

            encodedInstruction = encodeRType(ADD, r1, r2);
        }

        else if (strcmp(instruction, "LDI") == 0) {

            int r1 = atoi(operand1 + 1);
            int imm = atoi(operand2);

            encodedInstruction = encodeIType(LDI, r1, imm);
        }
        else if (strcmp(instruction, "SUB") == 0) {

    int r1 = atoi(operand1 + 1);
    int r2 = atoi(operand2 + 1);

    encodedInstruction = encodeRType(SUB, r1, r2);
}

else if (strcmp(instruction, "MUL") == 0) {

    int r1 = atoi(operand1 + 1);
    int r2 = atoi(operand2 + 1);

    encodedInstruction = encodeRType(MUL, r1, r2);
}

else if (strcmp(instruction, "AND") == 0) {

    int r1 = atoi(operand1 + 1);
    int r2 = atoi(operand2 + 1);

    encodedInstruction = encodeRType(AND, r1, r2);
}

else if (strcmp(instruction, "OR") == 0) {

    int r1 = atoi(operand1 + 1);
    int r2 = atoi(operand2 + 1);

    encodedInstruction = encodeRType(OR, r1, r2);
}

else if (strcmp(instruction, "JR") == 0) {

    int r1 = atoi(operand1 + 1);
    int r2 = atoi(operand2 + 1);

    encodedInstruction = encodeRType(JR, r1, r2);
}
else if (strcmp(instruction, "BEQZ") == 0) {

    int r1 = atoi(operand1 + 1);
    int imm = atoi(operand2);

    encodedInstruction = encodeIType(BEQZ, r1, imm);
}

else if (strcmp(instruction, "SAL") == 0) {

    int r1 = atoi(operand1 + 1);
    int imm = atoi(operand2);

    encodedInstruction = encodeIType(SAL, r1, imm);
}

else if (strcmp(instruction, "SAR") == 0) {

    int r1 = atoi(operand1 + 1);
    int imm = atoi(operand2);

    encodedInstruction = encodeIType(SAR, r1, imm);
}

else if (strcmp(instruction, "LB") == 0) {

    int r1 = atoi(operand1 + 1);
    int address = atoi(operand2);

    encodedInstruction = encodeIType(LB, r1, address);
}

else if (strcmp(instruction, "SB") == 0) {

    int r1 = atoi(operand1 + 1);
    int address = atoi(operand2);

    encodedInstruction = encodeIType(SB, r1, address);
}

        instructionMemory[instructionIndex] = encodedInstruction;

        instructionIndex++;
    }

    fclose(file);
}