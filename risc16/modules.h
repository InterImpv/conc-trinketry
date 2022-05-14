#include "error.h"

#include <stdint.h>

#define N_OF_REGS		8
#define MEM_CAPACITY	65536

typedef struct __mem_unit {
	uint16_t rom_ptr;
	uint16_t ram_ptr;

	uint16_t rom_beginp;
	uint16_t rom_endp;
	uint16_t ram_beginp;
	uint16_t ram_endp;

	uint16_t mem[MEM_CAPACITY];
} mem_unit;

typedef struct __reg_unit {
	uint16_t rx[N_OF_REGS];
	uint16_t pc;
} reg_unit;

enum RISC16 {
    __ADD	= 0,
    __ADDI	= 1,
    __NAND	= 2,
    __LUI	= 3,
    __SW	= 4,
    __LW	= 5,
    __BEQ	= 6,
    __JALR	= 7
};

/* instruction type */
typedef struct __instr {
	enum RISC16 opcode;
	uint8_t rA;
	uint8_t rB;
	uint8_t rC;
	uint16_t imm;
	uint16_t raw_data;
} instr_t;

/* instruction functions */
instr_t *instr_create(void);
enum GEN_ERR instr_free(instr_t *instr);

const uint16_t instr_fetch(mem_unit *mem, reg_unit *regs);
enum GEN_ERR instr_decode(instr_t *instr, uint16_t data);
enum GEN_ERR instr_exec(instr_t *instr, reg_unit *regs, mem_unit *mem);

enum GEN_ERR draw_instr(instr_t *instr);

/* register functions */
reg_unit *reg_create(void);
enum GEN_ERR reg_free(reg_unit *regs);
enum GEN_ERR reg_reset(reg_unit *regs);

void inc_pc(reg_unit *regs);
void set_pc(reg_unit *regs, uint16_t addr);

enum GEN_ERR draw_regs(reg_unit *regs);

/* memory functions */
mem_unit *mem_create(void);
enum GEN_ERR mem_free(mem_unit *mem);
enum GEN_ERR mem_reset(mem_unit *mem);
enum GEN_ERR mem_fill(mem_unit *mem, const char *filepath);

void inc_rom_ptr(mem_unit *mem);
void dec_rom_ptr(mem_unit *mem);
void inc_ram_ptr(mem_unit *mem);
void dec_ram_ptr(mem_unit *mem);

enum GEN_ERR draw_mem(mem_unit *mem);
enum GEN_ERR draw_stdout(mem_unit *mem);
