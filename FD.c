/* =======================================================================
 * FD.c — Fetch and Decode stages for Package 4
 *
 * fetch()  : reads instructionMemory[pc], fills IF_ID latch, PC++
 * decode() : copies IF_ID -> ID_EX, reads register file,
 *             runs hazard detection, applies forwarding.
 * ======================================================================= */
#include "defs.h"

/* ── fetch ───────────────────────────────────────────────────────────── */
void fetch(int cycle)
{
    short int instruction = instructionMemory[pc];
    uint16_t ipc = pc;
    pc++;

    IF_ID.valid = 1;
    IF_ID.pc_of_instr = (int)ipc;
    IF_ID.raw = instruction;
    IF_ID.opcode = (instruction >> 12) & 0xF;
    IF_ID.r1 = (instruction >> 6) & 0x3F;
    IF_ID.r2 = instruction & 0x3F;

    /* sign-extend 6-bit immediate */
    int raw_imm = instruction & 0x3F;
    IF_ID.imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F)
                                 : (int8_t)raw_imm;

    /* Snapshot register values (may be patched by forwarding in ID) */
    IF_ID.val_r1 = registerFile[IF_ID.r1];
    IF_ID.val_r2 = registerFile[IF_ID.r2];

    printf("[IF  | cycle %d] PC=%d  encoded=%u  opcode=%d  R%d  R%d  imm=%d\n",
           cycle, IF_ID.pc_of_instr, (unsigned short)instruction,
           IF_ID.opcode, IF_ID.r1, IF_ID.r2, (int)IF_ID.imm);
}

/* ── decode ──────────────────────────────────────────────────────────── */
void decode(int cycle)
{
    if (!IF_ID.valid)
    {
        /* bubble in IF/ID — pass a bubble through to ID/EX */
        printf("[ID  | cycle %d] (bubble)\n", cycle);
        ID_EX.valid = 0;
        return;
    }

    /* Detect hazard BEFORE advancing the latch.
     * detectHazard() checks IF_ID (incoming) vs ID_EX (currently in EX).
     * If a load-use stall is needed it sets stall_pipeline = 1.          */
    detectHazard();

    if (stall_pipeline)
    {
        /* Hold: do NOT advance IF_ID into ID_EX; insert bubble in ID/EX  */
        printf("[ID  | cycle %d] ** STALL ** holding R%d for load-use hazard\n",
               cycle, IF_ID.r1);
        ID_EX.valid = 0; /* bubble propagates into EX */
        /* IF_ID is NOT cleared — it will be decoded again next cycle     */
        /* Also undo the PC increment done in fetch() this cycle           */
        pc--;
        /* Clear the new IF_ID that was just fetched this cycle            */
        memset(&IF_ID, 0, sizeof(Instruction));
        return;
    }

    /* Normal advance: copy IF_ID into ID_EX */
    ID_EX = IF_ID;
    ID_EX.valid = 1;

    /* Fresh register read */
    ID_EX.val_r1 = registerFile[ID_EX.r1];
    ID_EX.val_r2 = registerFile[ID_EX.r2];

    /* Apply forwarding from previous EX result */
    applyForwarding(&ID_EX, cycle);

    printf("[ID  | cycle %d] PC=%d  opcode=%d  R%d(%d)  R%d(%d)  imm=%d\n",
           cycle, ID_EX.pc_of_instr, ID_EX.opcode,
           ID_EX.r1, (int)ID_EX.val_r1,
           ID_EX.r2, (int)ID_EX.val_r2,
           (int)ID_EX.imm);
}
