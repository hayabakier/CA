/* =======================================================================
 * defs.h  —  Single unified header for Package 4 pipeline simulator
 *
 * Package 4: Harvard, 3-stage pipeline (IF → ID → EX), 8-bit registers,
 *            16-bit instructions, circular shifts (SAL/SAR).
 *            Cycle count: 3 + (n-1)*1
 * ======================================================================= */

#ifndef DEFS_H
#define DEFS_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* ── Memory / register sizes ─────────────────────────────────────────── */
#define INSTR_MEM_SIZE   1024   /* 1024 x 16-bit words                  */
#define DATA_MEM_SIZE    2048   /* 2048 x  8-bit bytes                  */
#define REG_FILE_SIZE      64   /* R0..R63, all 8-bit                   */

/* ── Opcodes (4-bit, 0-11) ──────────────────────────────────────────── */
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

/* ── SREG flag bit positions ────────────────────────────────────────── */
#define FLAG_C  4
#define FLAG_V  3
#define FLAG_N  2
#define FLAG_S  1
#define FLAG_Z  0

/* ── Pipeline latch ─────────────────────────────────────────────────── */
typedef struct {
    int       valid;        /* 1 = real instruction, 0 = bubble/empty   */
    int       pc_of_instr;  /* PC of this instruction when fetched       */
    int       opcode;
    int       r1;           /* bits [11:6]                               */
    int       r2;           /* bits [ 5:0]                               */
    int8_t    imm;          /* sign-extended 6-bit immediate             */
    int8_t    val_r1;       /* value of r1 read in ID (after forwarding) */
    int8_t    val_r2;       /* value of r2 read in ID (after forwarding) */
    short int raw;          /* original 16-bit encoded instruction       */
} Instruction;

/* ── Global state – defined in exactly one .c each ─────────────────── */

/* execute.c owns registers + SREG + cycle counter + forwarding state   */
extern int8_t   registerFile[REG_FILE_SIZE];
extern uint16_t pc;
extern uint8_t  sreg;
extern int      current_cycle;
extern int8_t   fwd_result;    /* value produced by EX last cycle        */
extern int      fwd_dest_reg;  /* destination register of that result    */
extern int      fwd_valid;     /* 1 if forwarding is live                */

/* memory.c owns both memories                                           */
extern short int instructionMemory[INSTR_MEM_SIZE];
extern int8_t    dataMemory[DATA_MEM_SIZE];
extern int       NumberofInstructions;

/* pipeline.c owns the two inter-stage latches                           */
extern Instruction IF_ID;   /* between IF and ID                         */
extern Instruction ID_EX;   /* between ID and EX                         */

/* hazard.c owns the stall flag                                          */
extern int stall_pipeline;
extern int flush_pending;

/* ── Function prototypes ─────────────────────────────────────────────── */

/* parser.c */
short int encodeRType(int opcode, int r1, int r2);
short int encodeIType(int opcode, int r1, int imm);
void      parseFile(const char *filename);

/* FD.c */
void fetch(int cycle);
void decode(int cycle);

/* execute.c */
void execute(int cycle);

/* hazard.c */
int  detectHazard(void);   /* checks ID_EX vs IF_ID; returns 1 if stall needed */

/* memory_WB.c */
void applyForwarding(Instruction *latch, int cycle);
void printCycleState(int cycle);

#endif /* DEFS_H */
