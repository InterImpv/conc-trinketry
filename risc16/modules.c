#include "modules.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

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

	mem->ram_ptr = 0;
	memset(mem->rom, 0, MEM_SIZE * sizeof(*mem->rom));
	memset(mem->ram, 0, MEM_SIZE * sizeof(*mem->rom));

    return E_OK;
}

enum GEN_ERR mem_fill(mem_unit *mem, const char *filepath)
{
    assert(NULL != mem);
    FILE *f = fopen(filepath, "r");
    assert(NULL != f);

	char *line = NULL;
	size_t n = 0;
	uint16_t word = 0x0000;

	for (uint16_t i = 0; i < MEM_SIZE; i++) {
		if (getline(&line, &n, f) == -1)
			break;

		word = strtol(line, NULL, 16);
		mem->rom[i] = word;

		printf("0x%04x from %s", word, line);
	}

	free(line);
	fclose(f);

    return E_OK;
}

/* prints */
void print_mem(mem_unit *mem)
{
	printf("\n\tMEMORY MAP (ROM : RAM):\n\n");
	for (int i = 0; i < MEM_SIZE; i++) {
		printf("[0x%04x]: 0x%04x\t0x%04x\n", i, mem->rom[i], mem->ram[i]);
	}
}

void print_regs(reg_unit *regs)
{
	printf("\n\tREGISTERS:\n\n");
	for (int i = 0; i < N_OF_REGS; i++) {
		printf("$R%d: 0x%04x\n", i, regs->rx[i]);
	}
	printf("$PC: 0x%04x\n", regs->pc);
}
