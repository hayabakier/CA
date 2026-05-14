#include <stdio.h>
#include <string.h>
#include "pipeline.h"
#include "memory.h"
#include "parser.h"

extern int8_t registerFile[64]; // leave here to test
extern int pc;                  // leave here to test
extern int programSize;         // leave here to test

// extern void fetch(void);
// extern void decode(short int instruction);
// extern int ALU(int operandA, int operandB, int operation);
// extern void updateSREG(int result, int operandA, int operandB, int opcode);
// extern void memAccess(int opcode, int r1, int8_t imm);
// extern void writeBack(int r1, int8_t result);

Instruction IF_ID = {0};
Instruction ID_EX = {0};

static void insert_bubble(Instruction *latch)
{
    memset(latch, 0, sizeof(Instruction));
    latch->valid = 0;
}

static void stage_IF(void)
{
    if (pc >= programSize)
    {
        insert_bubble(&IF_ID);
        printf("  [IF]  (nothing to fetch)\n");
        return;
    }
    // Save pc BEFORE incrementing — needed for branch instructions
    IF_ID.pc_of_instr = pc;
    IF_ID.valid = 1;

    printf("  IF/ID : PC = %d  |  encoded = %d  |  binary = ", pc, (unsigned short)instructionMemory[pc]);

    short int raw = instructionMemory[pc];

    /* Extract latch fields from the raw 16-bit instruction */
    IF_ID.opcode = (raw >> 12) & 0xF;
    IF_ID.r1 = (raw >> 6) & 0x3F;
    IF_ID.r2 = raw & 0x3F;

    //
    int raw_imm = raw & 0x3F;
    IF_ID.imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F) : (int8_t)raw_imm;

    /* Read register values — needed by EX for BEQZ and JR */
    IF_ID.val_r1 = registerFile[IF_ID.r1];
    IF_ID.val_r2 = registerFile[IF_ID.r2];

    // fetch should be called here
    printf("  [IF]  PC = %d  |  encoded = %d\n", pc, (unsigned short)raw);
}

static void stage_ID(void)
{

    if (!IF_ID.valid)
    {
        insert_bubble(&ID_EX);
        printf("  [ID]  (bubble)\n");
        return;
    }

    short int raw = instructionMemory[pc - 1]; // pc was already incremented in fetch()
    decode_latch(raw, &ID_EX);                 // to put fields in ID_EX for EX to read
    // decode should be called here
    printf("  [ID]  opcode=%d  R%d(%d)  R%d(%d)  imm=%d\n", ID_EX.opcode, ID_EX.r1, (int)ID_EX.val_r1, ID_EX.r2, (int)ID_EX.val_r2, (int)ID_EX.imm);
}

static void decode_latch(short int instruction, Instruction *id_ex)
{
    id_ex->opcode = (instruction >> 12) & 0xF;
    id_ex->r1 = (instruction >> 6) & 0x3F;
    id_ex->r2 = instruction & 0x3F;

    // Sign-extend 6-bit immediate to int8_t
    int raw_imm = instruction & 0x3F;
    id_ex->imm = (raw_imm & 0x20) ? (int8_t)(raw_imm | ~0x3F)
                                  : (int8_t)raw_imm;

    // Read register values — needed by EX for BEQZ and JR
    id_ex->val_r1 = registerFile[id_ex->r1];
    id_ex->val_r2 = registerFile[id_ex->r2];
}

static void stage_EX(int *branch_target, int *branched)
{
    if (!ID_EX.valid)
    {
        printf("  [EX]  (bubble)\n");
        *branched = 0;
        return;
    }

    int opcode = ID_EX.opcode;
    int a = (int)ID_EX.val_r1; //  first operand
    int b = (int)ID_EX.val_r2; // second operand
    printf("  [EX]  opcode=%d  R%d(%d)  R%d(%d)  imm=%d\n", opcode, ID_EX.r1, a, ID_EX.r2, b, (int)ID_EX.imm);

    int alu_result = 0;

    // execution methods should be called here
    switch (opcode)
    {
    case ADD:
        // alu_result = ALU(a, b, 2);
        break;
    case SUB:
        // alu_result = ALU(a, b, 3);
        break;
    case MUL:
        //   alu_result = ALU(a, b, 4);
        break;
    case LDI:
        //  alu_result = (int)ID_EX.imm;
        break;
    case AND:
        //  alu_result = ALU(a, b, 0);
        break;
    case OR:
        //  alu_result = ALU(a, b, 1);
        break;
    case SAL:
        //  alu_result = ALU(a, (int)(uint8_t)ID_EX.imm, 5);
        break;
    case SAR:
        //  alu_result = ALU(a, (int)(uint8_t)ID_EX.imm, 6);
        break;
    case BEQZ:
        /*
            if (ID_EX.val_r1 == 0)
            {
                *branch_target = ID_EX.pc_of_instr + 1 + (int)ID_EX.imm;
                printf("  [EX]  BEQZ TAKEN → new PC = %d + 1 + (%d) = %d\n", ID_EX.pc_of_instr, (int)ID_EX.imm, *branch_target);
                *branched = 1;
            }
            else
            {
                printf("  [EX]  BEQZ NOT taken  (R%d = %d)\n", ID_EX.r1, (int)ID_EX.val_r1);
                *branched = 0;
            }
         */
        break;
    case JR:
        /*
            *branch_target = (((unsigned int)ID_EX.val_r1 & 0xFF) << 8) | ((unsigned int)ID_EX.val_r2 & 0xFF);
            printf("  [EX]  JR → R%d(0x%02X)||R%d(0x%02X) = new PC %d\n", ID_EX.r1, (unsigned int)ID_EX.val_r1 & 0xFF, ID_EX.r2, (unsigned int)ID_EX.val_r2 & 0xFF, *branch_target);
            *branched = 1;
            */
        break;
    case LB: // memAccess() will handle the load, we just need to call it for the side effect of printing
        break;
    case SB: // memAccess() will handle the store, we just need to call it for the side effect of printing
        break;
    default:
        printf("  [EX]  unknown opcode %d\n", opcode);
        break;
    }
    // ── Member 2: update SREG flags ──
    // updateSREG(alu_result, a, b, opcode);

    //  ── Member 6: data memory access (LB / SB) ──
    // memAccess(opcode, ID_EX.r1, ID_EX.imm);

    // ── Member 6: write result back to register file ──
    // writeBack(ID_EX.r1, (int8_t)(alu_result & 0xFF));
    return;
}

static void flush_IF_ID(void)
{
    insert_bubble(&IF_ID);
    printf("  [FLUSH] IF/ID cleared → bubble\n");
}

static void flush_ID_EX(void)
{
    insert_bubble(&ID_EX);
    printf("  [FLUSH] ID/EX cleared → bubble\n");
}

// END-OF-SIMULATION PRINTINGS
static void print_all_registers(void)
{
    printf("\n========== REGISTER FILE ==========\n");
    for (int i = 0; i < 64; i++)
    {
        printf("R%-2d = %4d  ", i, (int)registerFile[i]);
        if ((i + 1) % 8 == 0)
            printf("\n");
    }
    printf("PC = %d\n", pc);
    /* SREG printed by Member 2 */
}

static void print_instruction_memory(void)
{
    printf("\n========== INSTRUCTION MEMORY ==========\n");
    for (int i = 0; i < programSize; i++)
        printf("imem[%4d] = %d\n", i, (unsigned short)instructionMemory[i]);
}

static void print_data_memory(void)
{
    printf("\n========== DATA MEMORY ==========\n");
    int any = 0;
    for (int i = 0; i < 2048; i++)
    {
        if (dataMemory[i] != 0)
        {
            printf("dataMemory[%4d] = %d\n", i, (int)dataMemory[i]);
            any = 1;
        }
    }
    if (!any)
        printf("(all zeros)\n");
}

void runPipeline(void)
{
    int cycle = 0;
    // Pipeline starts empty
    insert_bubble(&IF_ID);
    insert_bubble(&ID_EX);
    while (1)
    {
        //  Stopping condition
        int active = (pc < programSize) || IF_ID.valid || ID_EX.valid;
        if (!active)
            break;
        cycle++;
        printf("\n========== Clock Cycle %d ==========\n", cycle);
        //  EX → ID → IF  (reverse order — see comment above)
        int branch_target = -1;
        int branched = 0;
        stage_EX(&branch_target, &branched);
        stage_ID();
        stage_IF();

        if (branched)
        {
            // flush needed
            flush_IF_ID();
            flush_ID_EX();
            if (branch_target >= 0 && branch_target < 1024)
            {
                pc = branch_target;
                printf("  [PC UPDATE] pc = %d\n", pc);
            }
            else
            {
                printf("  [ERROR] branch target %d out of range\n",
                       branch_target);
                break;
            }
        }
    }
    printf("\n========== Simulation Complete | Total Cycles: %d ==========\n", cycle);
    print_all_registers();
    print_instruction_memory();
    print_data_memory();
}