/* =======================================================================
 * main.c — Package 4 pipeline driver
 *
 * 3-stage Harvard pipeline: IF → ID → EX
 * One new instruction fetched per cycle (when available).
 * Stage order per cycle: EX → ID → IF
 *   • EX first: commits result + sets fwd_result + may flush latches
 *   • ID second: reads regs, applies forwarding (skipped if flushed)
 *   • IF third:  fetches next instruction into IF_ID
 * ======================================================================= */
#include "defs.h"

#include "memory.c"
#include "parser.c"
#include "pipeline.c"
#include "hazard.c"
#include "memory_WB.c"
#include "FD.c"
#include "execute.c"

int main(void)
{
    parseFile("program.txt");

    int cycle = 1;

    while (pc < (uint16_t)NumberofInstructions || IF_ID.valid || ID_EX.valid)
    {
        current_cycle = cycle;
        printf("\n========== Clock Cycle %d ==========\n", cycle);

        flush_pending = 0;

        /* ── EX stage ── */
        execute(cycle);   /* may set flush_pending and clear latches */

        if (!flush_pending) {
            /* ── ID stage ── */
            decode(cycle);

            /* ── IF stage ── */
            if (pc < (uint16_t)NumberofInstructions && !stall_pipeline)
                fetch(cycle);
            else if (pc >= (uint16_t)NumberofInstructions && !stall_pipeline)
                printf("[IF  | cycle %d] (no more instructions)\n", cycle);
        } else {
            /* Branch/jump flushed the pipeline — skip ID and IF this cycle */
            printf("[ID  | cycle %d] (flushed)\n", cycle);
            printf("[IF  | cycle %d] (flushed)\n", cycle);
        }

        printCycleState(cycle);
        cycle++;
    }

    printf("\n========== Final State ==========\n");

    printf("\nRegisters:\n");
    for (int i = 0; i < REG_FILE_SIZE; i++)
        printf("  R%-2d = %d\n", i, (int)registerFile[i]);
    printf("  PC   = %u\n", (unsigned)pc);
    printf("  SREG = %u  [C=%d V=%d N=%d S=%d Z=%d]\n",
           (unsigned)sreg,
           (sreg >> FLAG_C) & 1, (sreg >> FLAG_V) & 1,
           (sreg >> FLAG_N) & 1, (sreg >> FLAG_S) & 1,
           (sreg >> FLAG_Z) & 1);

    printf("\nInstruction Memory (non-zero entries):\n");
    for (int i = 0; i < INSTR_MEM_SIZE; i++)
        if (instructionMemory[i] != 0)
            printf("  instructionMemory[%d] = %d\n", i, (int)instructionMemory[i]);

    printf("\nData Memory (non-zero entries):\n");
    for (int i = 0; i < DATA_MEM_SIZE; i++)
        if (dataMemory[i] != 0)
            printf("  dataMemory[%d] = %d\n", i, (int)dataMemory[i]);

    return 0;
}
