#include "modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <ncurses.h>

/* module functions */
/* register file */
reg_unit *reg_create(void)
{
	reg_unit *regs = NULL;
	regs = malloc(sizeof(*regs));
	assert(NULL != regs);

	return regs;
}

enum GEN_ERR reg_free(reg_unit *regs)
{
    assert(NULL != regs);
    free(regs);

    return E_OK;
}

enum GEN_ERR reg_init(reg_unit *regs)
{
    assert(NULL != regs);

	regs->pc = 0;
	memset(regs->rx, 0, N_OF_REGS * sizeof(*regs->rx));

    return E_OK;
}

void inc_pc(reg_unit *regs)
{
	regs->pc++;
	return;
}

void set_pc(reg_unit *regs, uint16_t addr)
{
	regs->pc = addr;
	return;
}

/* memory */
mem_unit *mem_create(void)
{
	mem_unit *mem = NULL;
	mem = malloc(sizeof(*mem));
	assert(NULL != mem);

	return mem;
}

enum GEN_ERR mem_free(mem_unit *mem)
{
    assert(NULL != mem);
    free(mem);

    return E_OK;
}

enum GEN_ERR mem_init(mem_unit *mem)
{
    assert(NULL != mem);

	mem->rom_ptr = 0;
	mem->ram_ptr = 0;
	mem->rom_beginp = 0;
	mem->rom_endp = 22;
	mem->ram_beginp = 0;
	mem->ram_endp = 22;
	memset(mem->rom, 0, ROM_CAPACITY * sizeof(*mem->rom));
	memset(mem->ram, 0, RAM_CAPACITY * sizeof(*mem->rom));

    return E_OK;
}

void inc_rom_ptr(mem_unit *mem)
{
	if (mem->rom_endp < ROM_CAPACITY) {
		mem->rom_beginp++;
		mem->rom_endp++;
	}
	return;
}

void dec_rom_ptr(mem_unit *mem)
{
	if (mem->rom_beginp > 0) {
		mem->rom_beginp--;
		mem->rom_endp--;
	}
	return;
}

void inc_ram_ptr(mem_unit *mem)
{
	if (mem->ram_endp < RAM_CAPACITY) {
		mem->ram_beginp++;
		mem->ram_endp++;
	}
	return;
}

void dec_ram_ptr(mem_unit *mem)
{
	if (mem->ram_beginp > 0) {
		mem->ram_beginp--;
		mem->ram_endp--;
	}
	return;
}

enum GEN_ERR mem_fill_rom(mem_unit *mem, const char *filepath)
{
    assert(NULL != mem);
    FILE *f = fopen(filepath, "r");
    assert(NULL != f);

	char *line = NULL;
	size_t n = 0;
	uint16_t word = 0x0000;

	for (uint16_t i = 0; i < ROM_CAPACITY; i++) {
		if (getline(&line, &n, f) == -1)
			break;

		word = strtol(line, NULL, 16);
		mem->rom[i] = word;
	}

	free(line);
	fclose(f);

    return E_OK;
}

enum GEN_ERR mem_fill_ram(mem_unit *mem, const char *filepath)
{
    assert(NULL != mem);
    FILE *f = fopen(filepath, "r");
    assert(NULL != f);

	char *line = NULL;
	size_t n = 0;
	uint16_t word = 0x0000;

	for (uint16_t i = 0; i < RAM_CAPACITY; i++) {
		if (getline(&line, &n, f) == -1)
			break;

		word = strtol(line, NULL, 16);
		mem->ram[i] = word;
	}

	free(line);
	fclose(f);

    return E_OK;
}

/* draws */
/* both rom & ram must be unified */
void draw_rom(mem_unit *mem)
{
	mvprintw(Y_ROM, X_ROM, "ROM: %u %u", mem->rom_beginp, mem->rom_endp);
	uint32_t j = 1;
	for (uint32_t i = mem->rom_beginp; i < mem->rom_endp; i++) {
		if (i == mem->rom_ptr)
			mvprintw(Y_ROM + j, X_ROM, "[0x%04x]: 0x%04x<", i, mem->rom[i]);
		else
			mvprintw(Y_ROM + j, X_ROM, "[0x%04x]: 0x%04x", i, mem->rom[i]);

		j++;
	}
}

void draw_ram(mem_unit *mem)
{
	mvprintw(Y_RAM, X_RAM, "RAM: %u %u", mem->ram_beginp, mem->ram_endp);
	uint32_t j = 1;
	for (uint32_t i = mem->ram_beginp; i < mem->ram_endp; i++) {
		if (i == mem->ram_ptr)
			mvprintw(Y_RAM + j, X_RAM, "[0x%04x]: 0x%04x< %c", i, mem->ram[i], mem->ram[i]);
		else
			mvprintw(Y_RAM + j, X_RAM, "[0x%04x]: 0x%04x %c", i, mem->ram[i], mem->ram[i]);

		j++;
	}
}

void draw_stdout(mem_unit *mem)		// shit, rewrite
{
	mvprintw(Y_STDOUT - 1, X_STDOUT, "--------------------");
	for (uint32_t i = 0; i < STDOUT_H; i++) {
		mvaddch(Y_STDOUT + i, X_STDOUT - 1, '|');
		mvaddch(Y_STDOUT + i, X_STDOUT + STDOUT_W, '|');
	}
	for (uint32_t i = 0; i < STDOUT_H; i++) {
		for (uint32_t j = 0; j < STDOUT_W; j++) {
			uint16_t index = STDOUT_START + (STDOUT_W * i + j);

			if (isprint(mem->ram[index]))
				mvaddch(Y_STDOUT + i, X_STDOUT + j, mem->ram[index]);
		}
	}
	mvprintw(Y_STDOUT + STDOUT_H, X_STDOUT, "--------------------");
}

void draw_regs(reg_unit *regs)
{
	mvprintw(Y_REGS, X_REGS, "REGISTERS:");
	for (int i = 0; i < N_OF_REGS; i++) {
		mvprintw(Y_REGS + i + 1, X_REGS, "$R%d: 0x%04x", i, regs->rx[i]);
	}
	mvprintw(Y_REGS + N_OF_REGS + 1, X_REGS, "$PC: 0x%04x", regs->pc);
}
