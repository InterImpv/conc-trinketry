#include "modules.h"

#include <cstdint>
#include <list>

#define IOBUF_SIZE 33

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

enum CTRL_CMD {
	CMD_JMPTOMEM = 0,
	CMD_POKEMEM = 1
};

class instr_t {
private:
	const char *__op2str(enum RISC16 opcode);

public:
	enum RISC16 opcode;
	uint8_t rA;
	uint8_t rB;
	uint8_t rC;
	uint16_t imm;
	uint16_t raw_data;

	instr_t(void) = default;
	~instr_t(void) = default;

	enum GEN_ERR decode(const uint16_t data);
	void draw(const uint32_t ypos, const uint32_t xpos);
};

class ctrl_unit {
private:
	/* private members BEGIN */
	enum CTRL_CMD command;
	uint16_t arg_addr;
	uint16_t arg_data;

	uint16_t raw_data;

	std::size_t xscr_size;
	std::size_t yscr_size;

	std::string iobuf;

	mem_unit *mem = nullptr;
	reg_unit *reg = nullptr;
	instr_t instr;

	std::list<uint16_t> bpoints;
	/* private members END */
	/* private functions BEGIN */
	void __add(void);
	void __addi(void);
	void __nand(void);
	void __lui(void);
	void __sw(void);
	void __lw(void);
	void __beq(void);
	void __jalr(void);

	void flush_iobuf(void);
	enum GEN_ERR set_argaddr_from_str(const std::string str);
	enum GEN_ERR set_argdata_from_str(const std::string str);
	void cmd_jumptomem(void);
	void cmd_pokemem(void);
	void cmd_setpc(void);
	void cmd_exenticks(void);
	void cmd_addbreak(void);
	void cmd_delbreak(void);
	/* private functions END */
public:
	ctrl_unit(void) = default;
	~ctrl_unit(void) = default;

	enum GEN_ERR set_mem(mem_unit *mem);
	enum GEN_ERR set_reg(reg_unit *reg);
	void cmd_exetobreak(void);

	enum GEN_ERR fetch(void);
	enum GEN_ERR decode(void);
	enum GEN_ERR execute(void);

	enum GEN_ERR getline(void);
	enum GEN_ERR parseio(void);

	void draw(void);
};
