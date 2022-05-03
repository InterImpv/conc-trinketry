#include "instructions.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <ncurses.h>

const uint16_t OP_MASK = 0xe000;
const uint16_t RA_MASK = 0x1c00;
const uint16_t RB_MASK = 0x0380;
const uint16_t RC_MASK = 0x0007;
const uint16_t LUI_MASK  = 0xffc0;
const uint16_t IM7_MASK  = 0x007f;
const uint16_t IM10_MASK = 0x03ff;

/* instruction functions */
static void __add(reg_unit *regs, instr_t *instr)
{
	if (instr->rA)
		regs->rx[instr->rA] = regs->rx[instr->rB] + regs->rx[instr->rC];

	inc_pc(regs);
	return;
}

static void __addi(reg_unit *regs, instr_t *instr)
{
	if (instr->rA)
		regs->rx[instr->rA] = regs->rx[instr->rB] + instr->imm;

	inc_pc(regs);
	return;
}

static void __nand(reg_unit *regs, instr_t *instr)
{
	if (instr->rA)
		regs->rx[instr->rA] = ~(regs->rx[instr->rB] & regs->rx[instr->rC]);

	inc_pc(regs);
	return;
}

static void __lui(reg_unit *regs, instr_t *instr)
{
	if (instr->rA)
		regs->rx[instr->rA] = (instr->imm << 6) & LUI_MASK;

	inc_pc(regs);
	return;
}

static void __sw(reg_unit *regs, mem_unit *mem, instr_t *instr)
{
	mem->ram_ptr = (instr->imm + regs->rx[instr->rB]);
	mem->ram[mem->ram_ptr] = regs->rx[instr->rA];

	inc_pc(regs);
	return;
}

static void __lw(reg_unit *regs, mem_unit *mem, instr_t *instr)
{
	mem->ram_ptr = (instr->imm + regs->rx[instr->rB]);
	regs->rx[instr->rA] = mem->ram[mem->ram_ptr];

	inc_pc(regs);
	return;
}

static void __beq(reg_unit *regs, instr_t *instr)
{
	if(regs->rx[instr->rA] == regs->rx[instr->rB])
		set_pc(regs, (regs->pc + 1 + instr->imm) & IM7_MASK);
	else
		inc_pc(regs);

	return;
}

static void __jalr(reg_unit *regs, instr_t *instr)
{
	/* in case if rA == rB */
	uint16_t addr = regs->pc + 1;

	set_pc(regs, regs->rx[instr->rB]);
	if (instr->rA)
		regs->rx[instr->rA] = addr;

	return;
}

static const char *__op2str(enum RISC16 opcode)
{
	switch (opcode)
	{
	case __ADD:
		return "add";
	case __ADDI:
		return "addi";
	case __NAND:
		return "nand";
	case __LUI:
		return "lui";
	case __SW:
		return "sw";
	case __LW:
		return "lw";
	case __BEQ:
		return "beq";
	case __JALR:
		return "jalr";

	default:
		return "INV";
	};
}

instr_t *instr_create(void)
{
	instr_t *instr = NULL;
	instr = malloc(sizeof(*instr));
	assert(NULL != instr);

	return instr;
}

enum GEN_ERR instr_free(instr_t *instr)
{
    assert(NULL != instr);
    free(instr);

    return E_OK;
}

void draw_instr(instr_t *instr)
{
	assert(NULL != instr);

	mvprintw(Y_INSTR, X_INSTR, "INSTRUCTION: 0x%04x", instr->raw_data);
	switch (instr->opcode)
	{
	case __ADD:
	case __NAND:
		mvprintw(Y_INSTR + 1, X_INSTR, "%s $r%d, $r%d, $r%d",
			__op2str(instr->opcode), instr->rA, instr->rB, instr->rC);
		break;
	case __ADDI:
	case __SW:
	case __LW:
	case __BEQ:
	case __JALR:
		mvprintw(Y_INSTR + 1, X_INSTR, "%s $r%d, $r%d, %u",
			__op2str(instr->opcode), instr->rA, instr->rB, instr->imm);
		break;
	case __LUI:
		mvprintw(Y_INSTR + 1, X_INSTR, "%s $r%d, %u",
			__op2str(instr->opcode), instr->rA, instr->imm);
		break;

	default:
		mvprintw(Y_INSTR + 1, X_INSTR, "invalid");
		break;
	};
}

const uint16_t instr_fetch(mem_unit *mem, reg_unit *regs)
{
	uint16_t data = 0x0000;
	assert(NULL != mem);
	assert(NULL != regs);

	if (regs->pc >= ROM_CAPACITY)
		return data;

	data = mem->rom[regs->pc];
	mem->rom_ptr = regs->pc;

	return data;
}

enum GEN_ERR instr_decode(instr_t *instr, uint16_t data)
{
	assert(NULL != instr);
	enum GEN_ERR ret = E_OK;

	uint8_t rA = 0;
	uint8_t rB = 0;
	uint8_t rC = 0;
	uint16_t imm = 0;

	enum RISC16 opcode = (data & OP_MASK) >> 13;

	switch (opcode)
	{
	case __ADD:
	case __NAND:
		rA = (data & RA_MASK) >> 10;
		rB = (data & RB_MASK) >> 7;
		rC = data & RC_MASK;
		break;
	case __ADDI:
	case __SW:
	case __LW:
	case __BEQ:
	case __JALR:
		rA = (data & RA_MASK) >> 10;
		rB = (data & RB_MASK) >> 7;
		imm = data & IM7_MASK;
		break;
	case __LUI:
		rA = (data & RA_MASK) >> 10;
		imm = data & IM10_MASK;
		break;

	default:	/* nop */
		ret = E_ARG;
		break;
	};

	instr->opcode = opcode;
	instr->rA = rA;
	instr->rB = rB;
	instr->rC = rC;
	instr->imm = imm;
	instr->raw_data = data;

	return ret;
}

enum GEN_ERR instr_exec(instr_t *instr, reg_unit *regs, mem_unit *mem)
{
	assert(NULL != instr);

	enum GEN_ERR ret = E_OK;
	switch (instr->opcode)
	{
	case __ADD:
		__add(regs, instr);
		break;
	case __ADDI:
		__addi(regs, instr);
		break;
	case __NAND:
		__nand(regs, instr);
		break;
	case __LUI:
		__lui(regs, instr);
		break;
	case __SW:
		__sw(regs, mem, instr);
		break;
	case __LW:
		__lw(regs, mem, instr);
		break;
	case __BEQ:
		__beq(regs, instr);
		break;
	case __JALR:
		__jalr(regs, instr);
		break;

	default:
		ret = E_ARG;
		break;
	};

	return ret;
}
