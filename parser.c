#include "defs.h"

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

void parseFile(const char *filename) {

    FILE *file = fopen(filename, "r");

    if (file == NULL) {
        printf("Error opening file\n");
        return;
    }

    char line[100];
    int instructionIndex = 0;

    while (fgets(line, sizeof(line), file)) {

        int r1, r2, imm;
        short int encodedInstruction;
        int validInstruction = 1;

        if (sscanf(line, "ADD R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(ADD, r1, r2);
        }

        else if (sscanf(line, "SUB R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(SUB, r1, r2);
        }

        else if (sscanf(line, "MUL R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(MUL, r1, r2);
        }

        else if (sscanf(line, "AND R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(AND, r1, r2);
        }

        else if (sscanf(line, "OR R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(OR, r1, r2);
        }

        else if (sscanf(line, "JR R%d R%d", &r1, &r2) == 2) {
            encodedInstruction = encodeRType(JR, r1, r2);
        }

        else if (sscanf(line, "LDI R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(LDI, r1, imm);
        }

        else if (sscanf(line, "BEQZ R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(BEQZ, r1, imm);
        }

        else if (sscanf(line, "SAL R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(SAL, r1, imm);
        }

        else if (sscanf(line, "SAR R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(SAR, r1, imm);
        }

        else if (sscanf(line, "LB R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(LB, r1, imm);
        }

        else if (sscanf(line, "SB R%d %d", &r1, &imm) == 2) {
            encodedInstruction = encodeIType(SB, r1, imm);
        }

        else {
            validInstruction = 0;
        }

        if (validInstruction) {
            instructionMemory[instructionIndex] = encodedInstruction;
            instructionIndex++;
            NumberofInstructions++;
        }
    }

    fclose(file);
}