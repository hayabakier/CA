#define ADD 0
#define SUB 1
#define MUL 2
#define LDI 3
#define BEQZ 4
#define AND 5
#define OR 6
#define JR 7
#define SAL 8
#define SAR 9
#define LB 10
#define SB 11

#ifndef PARSER_H
#define PARSER_H

short int encodeRType(int opcode, int r1, int r2);
short int encodeIType(int opcode, int r1, int imm);
void parseFile(char *filename);
#endif