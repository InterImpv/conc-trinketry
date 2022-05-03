#include "error.h"
#include "winpos.h"

#include <stdint.h>

#define ROM_CAPACITY 32
#define RAM_CAPACITY 256
#define N_OF_REGS 8

#define STDOUT_START 0x0080
#define STDOUT_END 0x00f8
#define STDOUT_W 20
#define STDOUT_H 6
#define STDOUT_SIZE 120

typedef struct __mem_unit {
	uint16_t rom[ROM_CAPACITY];
	uint16_t ram[RAM_CAPACITY];

	uint16_t rom_ptr;
	uint16_t ram_ptr;

	uint16_t rom_beginp;
	uint16_t rom_endp;
	uint16_t ram_beginp;
	uint16_t ram_endp;
} mem_unit;

typedef struct __reg_unit {
	uint16_t rx[N_OF_REGS];
	uint16_t pc;
} reg_unit;

/* module functions */
reg_unit *reg_create(void);
enum GEN_ERR reg_free(reg_unit *regs);
enum GEN_ERR reg_init(reg_unit *regs);
void inc_pc(reg_unit *regs);
void set_pc(reg_unit *regs, uint16_t addr);

mem_unit *mem_create(void);
enum GEN_ERR mem_free(mem_unit *mem);
enum GEN_ERR mem_init(mem_unit *mem);
enum GEN_ERR mem_fill_rom(mem_unit *mem, const char *filepath);
enum GEN_ERR mem_fill_ram(mem_unit *mem, const char *filepath);

void inc_rom_ptr(mem_unit *mem);
void dec_rom_ptr(mem_unit *mem);
void inc_ram_ptr(mem_unit *mem);
void dec_ram_ptr(mem_unit *mem);

void draw_rom(mem_unit *mem);
void draw_ram(mem_unit *mem);
void draw_regs(reg_unit *regs);
void draw_stdout(mem_unit *mem);
