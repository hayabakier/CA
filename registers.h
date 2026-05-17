#ifndef REGISTERS_H
#define REGISTERS_H

#include <stdint.h>

extern int8_t registerFile[64];
extern short int pc;
extern uint8_t sreg;

#define FLAG_C 4
#define FLAG_V 3
#define FLAG_N 2
#define FLAG_S 1
#define FLAG_Z 0

#endif