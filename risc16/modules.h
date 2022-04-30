#include "error.h"

#include <stdint.h>

#define MEM_SIZE 20
#define N_OF_REGS 8

typedef struct __mem_unit {
	uint16_t rom[MEM_SIZE];
	uint16_t ram[MEM_SIZE];
	uint16_t ram_ptr;
} mem_unit;

typedef struct __reg_unit {
	uint16_t rx[N_OF_REGS];
	uint16_t pc;
} reg_unit;

/* instruction functions */
void inc_pc(reg_unit *regs);
void set_pc(reg_unit *regs, uint16_t addr);
void print_mem(mem_unit *mem);
void print_regs(reg_unit *regs);
