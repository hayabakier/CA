#include "defs.h"

/* ── Globals owned here ─────────────────────────────────────────────── */
int8_t registerFile[REG_FILE_SIZE] = {0};
short int pc = 0;
uint8_t sreg = 0;
int current_cycle = 0;
int flush_pending = 0;

/* Forwarding state */
int8_t fwd_result = 0;
int fwd_dest_reg = -1;
int fwd_valid = 0;

/* ── SREG helpers ────────────────────────────────────────────────────── */
static void sreg_guardian(void) { sreg &= 0x1F; }
static int get_flag(int bit) { return (sreg >> bit) & 1; }
static void set_flag(int bit, int val)
{
    if (val)
        sreg |= (uint8_t)(1u << bit);
    else
        sreg &= (uint8_t)~(1u << bit);
    sreg_guardian();
}
static void update_NZ(int8_t r)
{
    set_flag(FLAG_N, r < 0);
    set_flag(FLAG_Z, r == 0);
}
static void update_add_flags(int8_t a, int8_t b, int8_t r)
{
    unsigned usum = (unsigned)(uint8_t)a + (unsigned)(uint8_t)b;
    set_flag(FLAG_C, (usum & 0x100) != 0);
    set_flag(FLAG_V, ((a >= 0 && b >= 0 && r < 0) || (a < 0 && b < 0 && r >= 0)));
    update_NZ(r);
    set_flag(FLAG_S, get_flag(FLAG_N) ^ get_flag(FLAG_V));
}
static void update_sub_flags(int8_t a, int8_t b, int8_t r)
{
    set_flag(FLAG_V, ((a >= 0 && b < 0 && r < 0) || (a < 0 && b >= 0 && r >= 0)));
    update_NZ(r);
    set_flag(FLAG_S, get_flag(FLAG_N) ^ get_flag(FLAG_V));
}
static void print_sreg(void)
{
    printf("  SREG=");
    for (int i = 7; i >= 0; i--)
        printf("%d", (sreg >> i) & 1);
    printf(" [C=%d V=%d N=%d S=%d Z=%d]\n",
           get_flag(FLAG_C), get_flag(FLAG_V), get_flag(FLAG_N),
           get_flag(FLAG_S), get_flag(FLAG_Z));
}

/* ── execute ─────────────────────────────────────────────────────────── */
void execute(int cycle)
{
    flush_pending = 0;
    fwd_valid = 0;
    fwd_dest_reg = -1;

    if (!ID_EX.valid)
    {
        printf("[EX  | cycle %d] (bubble)\n", cycle);
        return;
    }

    int8_t opcode = ID_EX.opcode;
    int8_t r1 = ID_EX.r1;
    int8_t r2 = ID_EX.r2;
    int8_t imm = ID_EX.imm;
    int8_t vr1 = ID_EX.val_r1;
    int8_t vr2 = ID_EX.val_r2;
    int addr = imm & 0x3F;
    int8_t result = 0;
    int branched = 0;

    int itype = (opcode == LDI || opcode == BEQZ ||
                 opcode == SAL || opcode == SAR ||
                 opcode == LB || opcode == SB);

    if (itype)
        printf("[EX  | cycle %d] PC=%d  opcode=%d  R%d(%d)  imm=%d\n",
               cycle, ID_EX.pc_of_instr, opcode,
               r1, (int)vr1, (int)imm);
    else
        printf("[EX  | cycle %d] PC=%d  opcode=%d  R%d(%d)  R%d(%d)\n",
               cycle, ID_EX.pc_of_instr, opcode,
               r1, (int)vr1, r2, (int)vr2);

    switch (opcode)
    {
    case ADD:
        result = (int8_t)(vr1 + vr2);
        update_add_flags(vr1, vr2, result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] ADD: R%d = %d + %d = %d\n", cycle, r1, (int)vr1, (int)vr2, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case SUB:
        result = (int8_t)(vr1 - vr2);
        update_sub_flags(vr1, vr2, result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] SUB: R%d = %d - %d = %d\n", cycle, r1, (int)vr1, (int)vr2, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case MUL:
        result = (int8_t)(vr1 * vr2);
        update_NZ(result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] MUL: R%d = %d * %d = %d\n", cycle, r1, (int)vr1, (int)vr2, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case LDI:
        result = imm;
        registerFile[r1] = result;
        printf("[EX  | cycle %d] LDI: R%d = %d\n", cycle, r1, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case BEQZ:
        if (vr1 == 0)
        {
            pc = (short int)(ID_EX.pc_of_instr + 1 + imm);
            printf("[EX  | cycle %d] BEQZ taken: R%d=0 -> PC=%d\n", cycle, r1, (int)pc);
            branched = 1;
        }
        else
        {
            printf("[EX  | cycle %d] BEQZ not taken: R%d=%d\n", cycle, r1, (int)vr1);
        }
        break;

    case AND:
        result = (int8_t)(vr1 & vr2);
        update_NZ(result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] AND: R%d = %d & %d = %d\n", cycle, r1, (int)vr1, (int)vr2, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case OR:
        result = (int8_t)(vr1 | vr2);
        update_NZ(result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] OR: R%d = %d | %d = %d\n", cycle, r1, (int)vr1, (int)vr2, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;

    case JR:
        pc = (short int)(((uint8_t)vr1 << 8) | (uint8_t)vr2);
        printf("[EX  | cycle %d] JR: PC -> %d (R%d=0x%02X R%d=0x%02X)\n",
               cycle, (int)pc, r1, (uint8_t)vr1, r2, (uint8_t)vr2);
        branched = 1;
        break;

    case SAL:
    {
        int shift = (int)(imm & 0x3F);
        result = (int8_t)((uint8_t)vr1 << shift);
        update_NZ(result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] SAL: R%d = %d << %d = %d\n", cycle, r1, (int)vr1, shift, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;
    }
    case SAR:
    {
        int shift = (int)(imm & 0x3F);
        result = (int8_t)((int8_t)vr1 >> shift);
        update_NZ(result);
        registerFile[r1] = result;
        printf("[EX  | cycle %d] SAR: R%d = %d >> %d = %d\n", cycle, r1, (int)vr1, shift, (int)result);
        printf("  R%d changed to %d\n", r1, (int)result);
        fwd_result = result;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;
    }
    case LB:
    {
        int8_t loaded = dataMemory[addr];
        registerFile[r1] = loaded;
        printf("[EX  | cycle %d] LB: dataMemory[%d]=%d -> R%d\n", cycle, addr, (int)loaded, r1);
        printf("  R%d changed to %d\n", r1, (int)loaded);
        fwd_result = loaded;
        fwd_dest_reg = r1;
        fwd_valid = 1;
        break;
    }
    case SB:
        dataMemory[addr] = vr1;
        printf("[EX  | cycle %d] SB: R%d(%d) -> dataMemory[%d]\n", cycle, r1, (int)vr1, addr);
        printf("  dataMemory[%d] changed to %d\n", addr, (int)vr1);
        break;

    default:
        printf("[EX  | cycle %d] Unknown opcode: %d\n", cycle, opcode);
        break;
    }

    sreg_guardian();
    print_sreg();

    /* ── Clear the latch we just consumed ──────────────────────────── */
    memset(&ID_EX, 0, sizeof(Instruction));

    /* ── Control hazard flush ────────────────────────────────────────── */
    if (branched)
    {
        flush_pending = 1;
        printf("[FLUSH | cycle %d] branch/jump taken -> PC=%d"
               " — flushing IF/ID and ID/EX\n",
               cycle, (int)pc);
        memset(&IF_ID, 0, sizeof(Instruction));
        fwd_valid = 0;
    }
}