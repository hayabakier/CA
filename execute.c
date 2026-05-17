#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "memory.h"
#include "parser.h"
#include "registers.h"  


static void sreg_guardian(void) { //keeps 3 upper bits of sreg always 0
    sreg &= 0b00011111; 
}

static int get_flag(int bit) { //reads 1 specific bit of sreg
    return (sreg >> bit) & 1;
}

static void set_flag(int bit, int value) {
    if (value)
        sreg |= (uint8_t)(1u << bit); //sets bit to 1
    else
        sreg &= (uint8_t)~(1u << bit); //sets bit to 0

    sreg_guardian();
}

static void update_NZ(int8_t result) {
    set_flag(FLAG_N, result < 0); // N=1 if result is negative
    set_flag(FLAG_Z, result == 0); // Z=1 if result is zero
}

static void update_add_flags(int8_t oldR1, int8_t oldR2, int8_t result) {
    int unsignedSum = (oldR1 & 0b11111111) + (oldR2 & 0b11111111); //anding the bottom 8 bits with ones 3a4an lama ne extend le 32 bits lw el number negative mybwz4 el carry

    // C is updated every ADD instruction. Check bit 8 of the unsigned result.
    set_flag(FLAG_C, (unsignedSum & 0b100000000) != 0); //lw bit 8 be 1, C=1

    // V is updated every ADD/SUB. ADD overflow: same sign inputs, opposite sign result.
    
    set_flag(FLAG_V, ((oldR1 >= 0 && oldR2 >= 0 && result < 0) || //if both operands pos and result is neg or both operandsn are neg and result is pos, V=1
                      (oldR1 < 0 && oldR2 < 0 && result >= 0)));

    update_NZ(result);
    set_flag(FLAG_S, get_flag(FLAG_N) ^ get_flag(FLAG_V));
}

static void update_sub_flags(int8_t oldR1, int8_t oldR2, int8_t result) {

    set_flag(FLAG_V, ((oldR1 >= 0 && oldR2 < 0 && result < 0) ||
                      (oldR1 < 0 && oldR2 >= 0 && result >= 0)));

    update_NZ(result);
    set_flag(FLAG_S, get_flag(FLAG_N) ^ get_flag(FLAG_V));
}

static void print_sreg(void) {
    printf("SREG = %u [C=%d V=%d N=%d S=%d Z=%d]\n",
           sreg,
           get_flag(FLAG_C), get_flag(FLAG_V), get_flag(FLAG_N),
           get_flag(FLAG_S), get_flag(FLAG_Z));
}


void execute(int8_t opcode, int8_t r1, int8_t r2, int8_t imm, uint16_t instructionPC) {
 
    int8_t oldR1 = registerFile[r1];
    int8_t oldR2 = registerFile[r2];
    int8_t result = 0;
    int address = imm & 0b00111111; // LB/SB use the 6-bit ADDRESS field as an unsigned address.

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
        result = (int8_t)(oldR1 << (imm & 0x3F));
        registerFile[r1] = result;
        update_NZ(result);
        printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
        break;

    case SAR:
        result = (int8_t)(oldR1 >> (imm & 0x3F));
        registerFile[r1] = result;
        update_NZ(result);
        printf("R%d changed to %d in EX stage\n", r1, registerFile[r1]);
        break;

        case LB:
            if (address >= 0 && address < 64) {
                registerFile[r1] = dataMemory[address];
                printf("R%d changed to %d from dataMemory[%d] in EX stage\n",
                       r1, registerFile[r1], address);
            }
            break;

        case SB:
            if (address >= 0 && address < 64) {
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