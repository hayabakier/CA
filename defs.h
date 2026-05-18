
#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define INSTR_MEM_SIZE   1024                 
#define DATA_MEM_SIZE    2048 
#define REG_FILE_SIZE      64  

#define ADD   0
#define SUB   1
#define MUL   2
#define LDI   3
#define BEQZ  4
#define AND   5
#define OR    6
#define JR    7
#define SAL   8
#define SAR   9
#define LB   10
#define SB   11

#define FLAG_C  4
#define FLAG_V  3
#define FLAG_N  2
#define FLAG_S  1
#define FLAG_Z  0

// Pipeline latch 
typedef struct {
    int valid;              // 1 = real instruction, 0 = bubble/empty   
    short int pc_of_instr;  // PC of this instruction when fetched       
    int8_t   opcode;
    int8_t    r1;           // bits [11:6]                              
    int8_t    r2;           // bits [ 5:0]                              
    int8_t    imm;          // sign-extended 6-bit immediate           
    int8_t    val_r1;       // value of r1 read in ID (after forwarding)
    int8_t    val_r2;      // value of r2 read in ID (after forwarding) 
    short int raw;          // original 16-bit encoded instruction       
} Instruction;


extern int8_t   registerFile[REG_FILE_SIZE];
extern short int pc;
extern uint8_t  sreg;
extern int      current_cycle;
extern int8_t   fwd_result;   
extern int      fwd_dest_reg;  
extern int      fwd_valid;     


extern short int instructionMemory[INSTR_MEM_SIZE];
extern int8_t    dataMemory[DATA_MEM_SIZE];
extern int       NumberofInstructions;


extern Instruction IF_ID;   
extern Instruction ID_EX;  


extern int stall_pipeline;
extern int flush_pending;



//parser.c
short int encodeRType(int opcode, int r1, int r2);
short int encodeIType(int opcode, int r1, int imm);
void parseFile(const char *filename);

// FD.c 
void fetch(int cycle);
void decode(int cycle);

// execute.c
void execute(int cycle);

// hazard.c 
int  detectHazard(void);  

// memory_WB.c 
void applyForwarding(Instruction *latch, int cycle);
void printCycleState(int cycle);

#endif 
