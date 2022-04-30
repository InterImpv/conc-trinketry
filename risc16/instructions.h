#include "modules.h"

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
} instr_t;

/* instruction functions */
instr_t *instr_create(void);
enum GEN_ERR instr_free(instr_t *instr);

void instr_dec_fill(instr_t *instr, uint16_t data);
enum GEN_ERR instr_exec(instr_t *instr, reg_unit *regs, mem_unit *mem);
const uint16_t instr_fetch(mem_unit *mem, reg_unit *regs);

void print_instr(instr_t *instr);

