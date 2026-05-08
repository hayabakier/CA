#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "parser.h"

int8_t registerFile[64] = {0};
uint16_t pc = 0;
uint8_t sreg = 0;

#define FLAG_C 4
#define FLAG_V 3
#define FLAG_N 2
#define FLAG_S 1
#define FLAG_Z 0

static void sreg_guardian(void) {
    sreg &= 0x1F;  
}

static int get_flag(int bit) {
    return (sreg >> bit) & 1;
}

static void set_flag(int bit, int value) {
    if (value)
        sreg |= (uint8_t)(1u << bit);
    else
        sreg &= (uint8_t)~(1u << bit);

    sreg_guardian();
}

static void update_NZ(int8_t result) {
    set_flag(FLAG_N, result < 0);
    set_flag(FLAG_Z, result == 0);
}

static void update_S_from_NV(void) {
    set_flag(FLAG_S, get_flag(FLAG_N) ^ get_flag(FLAG_V));
}

static void update_add_flags(int8_t oldR1, int8_t oldR2, int8_t result) {
    int unsignedSum = (oldR1 & 0xFF) + (oldR2 & 0xFF);

    // C is updated every ADD instruction. Check bit 8 of the unsigned result.
    set_flag(FLAG_C, (unsignedSum & 0x100) != 0);

    // V is updated every ADD/SUB. ADD overflow: same sign inputs, opposite sign result.
    set_flag(FLAG_V, ((oldR1 >= 0 && oldR2 >= 0 && result < 0) ||
                      (oldR1 < 0 && oldR2 < 0 && result >= 0)));

    update_NZ(result);
    update_S_from_NV();
}

static void update_sub_flags(int8_t oldR1, int8_t oldR2, int8_t result) {

    set_flag(FLAG_V, ((oldR1 >= 0 && oldR2 < 0 && result < 0) ||
                      (oldR1 < 0 && oldR2 >= 0 && result >= 0)));

    update_NZ(result);
    update_S_from_NV();
}

static void print_sreg(void) {
    printf("SREG = %u [C=%d V=%d N=%d S=%d Z=%d]\n",
           sreg,
           get_flag(FLAG_C), get_flag(FLAG_V), get_flag(FLAG_N),
           get_flag(FLAG_S), get_flag(FLAG_Z));
}

static int8_t sign_extend_6bit(short int value) {
    int8_t imm = (int8_t)(value & 0x3F);
    if (imm & 0x20)
        imm |= (int8_t)0xC0;
    return imm;
}

static short int get_opcode(short int instruction) {
    return (instruction >> 12) & 0xF;
}

static short int get_r1(short int instruction) {
    return (instruction >> 6) & 0x3F;
}

static short int get_r2(short int instruction) {
    return instruction & 0x3F;
}

static int8_t rotate_left_8(int8_t value, int amount) {
    uint8_t x = (uint8_t)value;
    amount %= 8;
    return (int8_t)((x << amount) | (x >> (8 - amount)));
}

static int8_t rotate_right_8(int8_t value, int amount) {
    uint8_t x = (uint8_t)value;
    amount %= 8;
    return (int8_t)((x >> amount) | (x << (8 - amount)));
}

void execute(short int instruction, uint16_t instructionPC) {
    short int opcode = get_opcode(instruction);
    short int r1 = get_r1(instruction);
    short int r2 = get_r2(instruction);          // R-format operand
    int8_t imm = sign_extend_6bit(instruction);  // I-format immediate/address

    int8_t oldR1 = registerFile[r1];
    int8_t oldR2 = registerFile[r2];
    int8_t result = 0;
    int address = imm & 0x3F; // LB/SB use the 6-bit ADDRESS field as an unsigned address.

    printf("Execute instruction: opcode=%d r1=R%d r2=R%d imm=%d\n", opcode, r1, r2, imm);

    switch (opcode) {
        case ADD:
            result = (int8_t)(oldR1 + oldR2);
            registerFile[r1] = result;
            update_add_flags(oldR1, oldR2, result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case SUB:
            result = (int8_t)(oldR1 - oldR2);
            registerFile[r1] = result;
            update_sub_flags(oldR1, oldR2, result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case MUL:
            result = (int8_t)(oldR1 * oldR2);
            registerFile[r1] = result;
            update_NZ(result);          // MUL affects N and Z only from these flags.
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case LDI:
            registerFile[r1] = imm;
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case BEQZ:
            if (oldR1 == 0) {
                pc = (uint16_t)(instructionPC + 1 + imm);
                printf("BEQZ taken: PC changed to %u in EX stage\n", pc);
            } else {
                printf("BEQZ not taken\n");
            }
            break;

        case AND:
            result = (int8_t)(oldR1 & oldR2);
            registerFile[r1] = result;
            update_NZ(result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case OR:
            result = (int8_t)(oldR1 | oldR2);
            registerFile[r1] = result;
            update_NZ(result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case JR:
            pc = (uint16_t)(((uint8_t)oldR1 << 8) | (uint8_t)oldR2);
            printf("JR: PC changed to %u in EX stage\n", pc);
            break;

        case SAL:
            result = rotate_left_8(oldR1, imm & 0x3F);
            registerFile[r1] = result;
            update_NZ(result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case SAR:
            result = rotate_right_8(oldR1, imm & 0x3F);
            registerFile[r1] = result;
            update_NZ(result);
            printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
            break;

        case LB:
            if (address >= 0 && address < 2048) {
                registerFile[r1] = dataMemory[address];
                printf("R%d changed to %d from dataMemory[%d] in EX stage\n",
                       r1, registerFile[r1], address);
            }
            break;

        case SB:
            if (address >= 0 && address < 2048) {
                dataMemory[address] = oldR1;
                printf("dataMemory[%d] changed to %d in EX stage\n", address, dataMemory[address]);
            }
            break;

        default:
            printf("Unknown opcode: %d\n", opcode);
            break;
    }

    sreg_guardian();
    print_sreg();
    printf("----------\n");
}

void decode(short int instruction, uint16_t instructionPC) {
    short int opcode = get_opcode(instruction);
    short int r1 = get_r1(instruction);
    short int r2 = get_r2(instruction);
    int8_t imm = sign_extend_6bit(instruction);

    printf("Decode instruction at PC=%u\n", instructionPC);
    printf("opcode = %d\n", opcode);
    printf("r1 = R%d, value = %d\n", r1, registerFile[r1]);
    printf("r2 = R%d, value = %d\n", r2, registerFile[r2]);
    printf("immediate/address = %d\n", imm);

    execute(instruction, instructionPC);
}

void fetch(void) {
    uint16_t instructionPC = pc;
    short int instruction = instructionMemory[pc];

    printf("Fetched instruction at PC=%u\n", pc);
    pc++;

    decode(instruction, instructionPC);
}

static int countProgramInstructions(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL)
        return 0;

    int count = 0;
    char line[100];
    while (fgets(line, sizeof(line), file)) {
        if (strtok(line, " \t\n") != NULL)
            count++;
    }

    fclose(file);
    return count;
}

static void print_all_registers(void) {
    printf("\nFinal Registers:\n");
    for (int i = 0; i < 64; i++) {
        printf("R%d = %d\n", i, registerFile[i]);
    }
    printf("PC = %u\n", pc);
    print_sreg();
}

int main(void) {
    const char *filename = "program.txt";
    int instructionCount = countProgramInstructions(filename);

    parseFile((char *)filename);

    while (pc < instructionCount) {
        fetch();
    }

    print_all_registers();
    return 0;
}
