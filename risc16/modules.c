#include "modules.h"
#include "winpos.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <ncurses.h>

static const uint32_t STDOUT_W		= 32;
static const uint32_t STDOUT_H		= 8;

static const uint16_t MASK_OP	= 0xe000;
static const uint16_t MASK_RA	= 0x1c00;
static const uint16_t MASK_RB	= 0x0380;
static const uint16_t MASK_RC	= 0x0007;
static const uint16_t MASK_LUI	= 0xffc0;
static const uint16_t MASK_IM7	= 0x007f;
static const uint16_t MASK_IM10	= 0x03ff;

/* UNSAFE instruction functions */
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
		regs->rx[instr->rA] = (instr->imm << 6) & MASK_LUI;

	inc_pc(regs);
	return;
}

static void __sw(reg_unit *regs, mem_unit *mem, instr_t *instr)
{
	mem->ram_ptr = (instr->imm + regs->rx[instr->rB]);
	if (mem->ram_ptr >= RAM_START && mem->ram_ptr < RAM_END)
		mem->mem[mem->ram_ptr] = regs->rx[instr->rA];

	inc_pc(regs);
	return;
}

static void __lw(reg_unit *regs, mem_unit *mem, instr_t *instr)
{
	mem->ram_ptr = (instr->imm + regs->rx[instr->rB]);
	regs->rx[instr->rA] = mem->mem[mem->ram_ptr];

	inc_pc(regs);
	return;
}

static void __beq(reg_unit *regs, instr_t *instr)
{
	if(regs->rx[instr->rA] == regs->rx[instr->rB])
		set_pc(regs, (regs->pc + 1 + instr->imm) & MASK_IM7);
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

/* instruction container */
instr_t *instr_create(void)
{
	instr_t *instr = NULL;
	instr = malloc(sizeof(*instr));
	return instr;
}

enum GEN_ERR instr_free(instr_t *instr)
{
    if (NULL == instr)
		return E_ARG;

	free(instr);
    return E_OK;
}

enum GEN_ERR draw_instr(instr_t *instr)
{
	if (NULL == instr)
		return E_ARG;

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

	return E_OK;
}

const uint16_t instr_fetch(mem_unit *mem, reg_unit *regs)
{
	uint16_t data = 0x0000;
	if (NULL == mem || NULL == regs)
		return data;

	if (regs->pc >= ROM_END)
		return data;

	data = mem->mem[regs->pc];
	mem->rom_ptr = regs->pc;

	return data;
}

enum GEN_ERR instr_decode(instr_t *instr, uint16_t data)
{
	enum GEN_ERR ret = E_OK;
	if (NULL == instr) {
		ret = E_ARG;
		return ret;
	}

	uint8_t rA = 0;
	uint8_t rB = 0;
	uint8_t rC = 0;
	uint16_t imm = 0;
	enum RISC16 opcode = (data & MASK_OP) >> 13;

	switch (opcode)
	{
	case __ADD:
	case __NAND:
		rA = (data & MASK_RA) >> 10;
		rB = (data & MASK_RB) >> 7;
		rC = data & MASK_RC;
		break;
	case __ADDI:
	case __SW:
	case __LW:
	case __BEQ:
	case __JALR:
		rA = (data & MASK_RA) >> 10;
		rB = (data & MASK_RB) >> 7;
		imm = data & MASK_IM7;
		break;
	case __LUI:
		rA = (data & MASK_RA) >> 10;
		imm = data & MASK_IM10;
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
	enum GEN_ERR ret = E_OK;
	if (NULL == instr) {
		ret = E_ARG;
		return ret;
	}

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

/* register file */
reg_unit *reg_create(void)
{
	reg_unit *regs = NULL;
	regs = malloc(sizeof(*regs));
	return regs;
}

enum GEN_ERR reg_free(reg_unit *regs)
{
    if (NULL == regs)
		return E_ARG;

    free(regs);
    return E_OK;
}

enum GEN_ERR reg_reset(reg_unit *regs)
{
    if (NULL == regs)
		return E_ARG;

	regs->pc = 0;
	memset(regs->rx, 0, N_OF_REGS * sizeof(*regs->rx));
    return E_OK;
}

void inc_pc(reg_unit *regs)
{
	if (NULL != regs)
		regs->pc++;

	return;
}

void set_pc(reg_unit *regs, uint16_t addr)
{
	if (NULL != regs && addr < ROM_END)
		regs->pc = addr;

	return;
}

/* memory */
mem_unit *mem_create(void)
{
	mem_unit *mem = NULL;
	mem = malloc(sizeof(*mem));
	return mem;
}

enum GEN_ERR mem_free(mem_unit *mem)
{
    if (NULL == mem)
		return E_ARG;

    free(mem);
    return E_OK;
}

enum GEN_ERR mem_reset(mem_unit *mem)
{
    if (NULL == mem)
		return E_ARG;

	mem->rom_ptr = 0;
	mem->ram_ptr = 0;
	mem->rom_beginp = ROM_START;
	mem->rom_endp = ROM_START + VIEW_MEM_RANGE;
	mem->ram_beginp = RAM_START;
	mem->ram_endp = RAM_START + VIEW_MEM_RANGE;
	memset(mem->mem, 0, MEM_CAPACITY * sizeof(*mem->mem));
    return E_OK;
}

void inc_rom_ptr(mem_unit *mem)
{
	if (NULL == mem)
		return;
	if (mem->rom_endp < ROM_END) {
		mem->rom_beginp++;
		mem->rom_endp++;
	}
	return;
}

void dec_rom_ptr(mem_unit *mem)
{
	if (NULL == mem)
		return;
	if (mem->rom_beginp > ROM_START) {
		mem->rom_beginp--;
		mem->rom_endp--;
	}
	return;
}

void inc_ram_ptr(mem_unit *mem)
{
	if (NULL == mem)
		return;
	if (mem->ram_endp < RAM_END) {
		mem->ram_beginp++;
		mem->ram_endp++;
	}
	return;
}

void dec_ram_ptr(mem_unit *mem)
{
	if (NULL == mem)
		return;
	if (mem->ram_beginp > RAM_START) {
		mem->ram_beginp--;
		mem->ram_endp--;
	}
	return;
}

enum GEN_ERR mem_fill(mem_unit *mem, const char *filepath)
{
    if (NULL == mem)
		return E_ARG;

    FILE *f = fopen(filepath, "r");
    if (NULL == f) {
		fprintf(stderr, "ERR %d: no such file \"%s\"\n", -E_IO, filepath);
		return E_IO;
	}

	char *line = NULL;
	size_t n = 0;
	uint16_t word = 0x0000;

	for (uint16_t i = 0; i < ROM_END; i++) {
		if (getline(&line, &n, f) == -1)
			break;

		word = strtol(line, NULL, 16);
		mem->mem[i] = word;
	}
	free(line);
	fclose(f);
    return E_OK;
}

/* draws */
static void rectangle(uint32_t y1, uint32_t x1, uint32_t y2, uint32_t x2)
{
    mvhline(y1, x1, '=', x2 - x1);
    mvhline(y2, x1, '=', x2 - x1);
    mvvline(y1, x1, '|', y2 - y1);
    mvvline(y1, x2, '|', y2 - y1);
    mvaddch(y1, x1, '+');
    mvaddch(y2, x1, '+');
    mvaddch(y1, x2, '+');
    mvaddch(y2, x2, '+');
}

static enum GEN_ERR draw_memseg(mem_unit *mem, uint32_t xpos, uint32_t ypos, uint16_t start, uint16_t end, uint16_t pos)
{
	if (NULL == mem)
		return E_ARG;

	mvprintw(ypos, xpos, "@: 0x%04x 0x%04x", start, end);
	uint32_t i, j;
	for (i = start, j = 1; i <= end; i++, j++) {
		if (i == pos)
			attron(A_BOLD);

		if (isprint(mem->mem[i]) && isascii(mem->mem[i])) {
			mvprintw(ypos + j, xpos, "[0x%04x]: 0x%04x %c",
				i, mem->mem[i], mem->mem[i]);
		} else {
			mvprintw(ypos + j, xpos, "[0x%04x]: 0x%04x",
				i, mem->mem[i], mem->mem[i]);
		}
		attroff(A_BOLD);
	}
	return E_OK;
}

enum GEN_ERR draw_mem(mem_unit *mem)
{
	if (NULL == mem)
		return E_ARG;

	draw_memseg(mem, X_ROM, Y_ROM, mem->rom_beginp, mem->rom_endp, mem->rom_ptr);
	draw_memseg(mem, X_RAM, Y_RAM, mem->ram_beginp, mem->ram_endp, mem->ram_ptr);
	return E_OK;
}

enum GEN_ERR draw_stdout(mem_unit *mem)
{
	if (NULL == mem)
		return E_ARG;

	attron(A_BOLD);
	rectangle(Y_STDOUT - 1, X_STDOUT - 1, Y_STDOUT + STDOUT_H, X_STDOUT + STDOUT_W);
	attroff(A_BOLD);

	for (uint32_t i = 0; i < STDOUT_H; i++) {
		for (uint32_t j = 0; j < STDOUT_W; j++) {
			uint16_t index = STDOUT_START + (STDOUT_W * i + j);

			if (isprint(mem->mem[index])) {
				mvaddch(Y_STDOUT + i, X_STDOUT + j, mem->mem[index]);
			}
		}
	}
	return E_OK;
}

enum GEN_ERR draw_regs(reg_unit *regs)
{
	if (NULL == regs)
		return E_ARG;

	mvprintw(Y_REGS, X_REGS, "REGISTERS:");
	for (int i = 0; i < N_OF_REGS; i++) {
		mvprintw(Y_REGS + i + 1, X_REGS, "$R%d: 0x%04x", i, regs->rx[i]);
	}
	mvprintw(Y_REGS + N_OF_REGS + 1, X_REGS, "$PC: 0x%04x", regs->pc);
	return E_OK;
}
