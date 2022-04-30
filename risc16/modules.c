#include "modules.h"

#include <stdint.h>
#include <stdio.h>

/* instruction functions */
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
