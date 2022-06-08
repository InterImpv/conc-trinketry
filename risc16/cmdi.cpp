#include "cmdi.h"
#include "gen-err.h"
#include "winpos.h"

#include <cstdint>
#include <algorithm>
#include <string>
#include <vector>
#include <ncurses.h>

static const uint16_t MASK_OP =		0xe000;
static const uint16_t MASK_RA =		0x1c00;
static const uint16_t MASK_RB =		0x0380;
static const uint16_t MASK_RC =		0x0007;
static const uint16_t MASK_LUI =	0xffc0;
static const uint16_t MASK_IM7 =	0x007f;
static const uint16_t MASK_IM10 =	0x03ff;

static uint16_t clamp_ui16(uint16_t val, uint16_t min, uint16_t max)
{
	const uint32_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

/* instruction struct interface BEGIN */
enum GEN_ERR instr_t::decode(const uint16_t data)
{
	enum GEN_ERR retval = E_OK;

	uint8_t rA = 0;
	uint8_t rB = 0;
	uint8_t rC = 0;
	uint16_t imm = 0;
	enum RISC16 opcode = static_cast<enum RISC16>((data & MASK_OP) >> 13);

	switch (opcode) {
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

	default:
		retval = E_ARG;
		break;
	};
	this->opcode = opcode;
	this->rA = rA;
	this->rB = rB;
	this->rC = rC;
	this->imm = imm;
	this->raw_data = data;

	return retval;
}

const char *instr_t::__op2str(enum RISC16 opcode)
{
	switch (opcode) {
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

void instr_t::draw(const uint32_t ypos, const uint32_t xpos)
{
	switch (this->opcode) {
	case __ADD:
	case __NAND:
		mvprintw(ypos + 1, xpos, "%s $r%d, $r%d, $r%d", __op2str(this->opcode), this->rA, this->rB, this->rC);
		break;
	case __ADDI:
	case __SW:
	case __LW:
	case __BEQ:
	case __JALR:
		mvprintw(ypos + 1, xpos, "%s $r%d, $r%d, %u", __op2str(this->opcode), this->rA, this->rB, this->imm);
		break;
	case __LUI:
		mvprintw(ypos + 1, xpos, "%s $r%d, %u", __op2str(this->opcode), this->rA, this->imm);
		break;

	default:
		mvprintw(ypos + 1, xpos, "invalid");
		break;
	};
	return;
}
/* instruction struct interface END */

/* control unit interface BEGIN */
	/* UNSAFE instruction functions BEGIN*/
void ctrl_unit::__add(void)
{
	if (instr.rA)
		reg->write(instr.rA, reg->read(instr.rB) + reg->read(instr.rC));

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__addi(void)
{
	if (instr.rA)
		reg->write(instr.rA, reg->read(instr.rB) + instr.imm);

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__nand(void)
{
	if (instr.rA)
		reg->write(instr.rA, ~(reg->read(instr.rB) & reg->read(instr.rC)));

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__lui(void)
{
	if (instr.rA)
		reg->write(instr.rA, (instr.imm << 6) & MASK_LUI);

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__sw(void)
{
	mem->ram_ptr = (instr.imm + reg->read(instr.rB));
	if (mem->ram_ptr >= RAM_START && mem->ram_ptr < RAM_END)
		mem->write(mem->ram_ptr, reg->read(instr.rA), false);

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__lw(void)
{
	mem->ram_ptr = (instr.imm + reg->read(instr.rB));
	reg->write(instr.rA, mem->read(mem->ram_ptr));

	mem->rom_ptr = reg->get_pc();
	reg->inc_pc();
	return;
}

void ctrl_unit::__beq(void)
{
	mem->rom_ptr = reg->get_pc();
	if (reg->read(instr.rA) == reg->read(instr.rB))
		reg->set_pc((reg->get_pc() + 1 + instr.imm) & MASK_IM7);
	else
		reg->inc_pc();

	return;
}

void ctrl_unit::__jalr(void)
{
	/* in case if rA == rB */
	uint16_t addr = reg->get_pc() + 1;
	mem->rom_ptr = reg->get_pc();

	reg->set_pc(reg->read(instr.rB));
	if (instr.rA)
		reg->write(instr.rA, addr);

	return;
}
	/* UNSAFE instruction functions END */

enum GEN_ERR ctrl_unit::set_mem(mem_unit *mem)
{
	if (!mem)
		return E_ARG;

	this->mem = mem;
	return E_OK;
}

enum GEN_ERR ctrl_unit::set_reg(reg_unit *reg)
{
	if (!reg)
		return E_ARG;

	this->reg = reg;
	return E_OK;
}

enum GEN_ERR ctrl_unit::fetch(void)
{
	this->raw_data = this->mem->read(this->reg->get_pc());
	return E_OK;
}

enum GEN_ERR ctrl_unit::decode(void)
{
	if (this->instr.decode(this->raw_data) != E_OK)
		return E_ARG;

	return E_OK;
}

enum GEN_ERR ctrl_unit::execute(void)
{
	enum GEN_ERR retval = E_OK;

	switch (instr.opcode) {
	case __ADD:
		__add();
		break;
	case __ADDI:
		__addi();
		break;
	case __NAND:
		__nand();
		break;
	case __LUI:
		__lui();
		break;
	case __SW:
		__sw();
		break;
	case __LW:
		__lw();
		break;
	case __BEQ:
		__beq();
		break;
	case __JALR:
		__jalr();
		break;

	default:
		retval = E_ARG;
		break;
	};
	return retval;
}

void ctrl_unit::flush_iobuf(void)
{
	this->iobuf.clear();
	return;
}

enum GEN_ERR ctrl_unit::set_argaddr_from_str(const std::string str)
{
	enum GEN_ERR retval = E_OK;
	try {
		this->arg_addr = std::stoi(str, nullptr, 16);
	} catch(std::invalid_argument const &ex) {
		this->iobuf = std::string("err: invalid address");
		retval = E_IO;
		return retval;
	} catch(std::out_of_range const &ex) {	// incorrect
		this->iobuf = std::string("err: address out of range");
		retval = E_RANGE;
		return retval;
	}
	return retval;
}

enum GEN_ERR ctrl_unit::set_argdata_from_str(const std::string str)
{
	enum GEN_ERR retval = E_OK;
	try {
		this->arg_data = std::stoi(str, nullptr, 16);
	} catch(std::invalid_argument const& ex) {
		this->iobuf = std::string("err: invalid data");
		retval = E_IO;
		return retval;
	} catch(std::out_of_range const& ex) {	// incorrect
		this->iobuf = std::string("err: data out of range");
		retval = E_RANGE;
		return retval;
	}
	return retval;
}

void ctrl_unit::cmd_jumptomem(void)
{
	uint16_t addr = this->arg_addr;
	if (addr >= ROM_START && addr <= ROM_END) {
		uint16_t addr_end = clamp_ui16(addr, ROM_START + VIEW_MEM_RANGE, ROM_END);
		uint16_t addr_begin = addr_end - VIEW_MEM_RANGE;

		this->mem->set_rom_beginp(addr_begin);
		this->mem->set_rom_endp(addr_end);
	}
	if (addr >= RAM_START && addr <= RAM_END) { // !
		uint16_t addr_end = clamp_ui16(addr, RAM_START + VIEW_MEM_RANGE, RAM_END);
		uint16_t addr_begin = addr_end - VIEW_MEM_RANGE;

		this->mem->set_ram_beginp(addr_begin);
		this->mem->set_ram_endp(addr_end);
	}
	return;
}

void ctrl_unit::cmd_pokemem(void)
{
	this->mem->write(this->arg_addr, this->arg_data, true);
	return;
}

void ctrl_unit::cmd_setpc(void)
{
	this->reg->set_pc(this->arg_addr);
	return;
}

void ctrl_unit::cmd_exenticks(void)
{
	uint16_t ticks = this->arg_data;
	while (ticks-- > 0) {
		this->fetch();
		this->decode();
		this->execute();
	}
	return;
}

void ctrl_unit::cmd_addbreak(void)
{
	this->bpoints.push_back(this->arg_addr);
	this->bpoints.unique();
	return;
}

void ctrl_unit::cmd_delbreak(void)
{
	this->bpoints.remove(this->arg_addr);
	return;
}

void ctrl_unit::cmd_exetobreak(void)
{
	int32_t key = ERR;
	std::list<uint16_t>::iterator it;

	timeout(0);	// non-blocking
	while ((key = getch()) == ERR) {
		it = std::find(this->bpoints.begin(), this->bpoints.end(), this->reg->get_pc());
		if(it != this->bpoints.end())
			break;

		this->fetch();
		this->decode();
		this->execute();
	}
	timeout(-1);	// blocking
	return;
}

enum GEN_ERR ctrl_unit::getline(void)
{
	enum GEN_ERR retval = E_OK;

	char buf[IOBUF_SIZE];
	this->flush_iobuf();
	/* echo guard begin */
	echo();
	curs_set(TRUE);

	/* input */
	/* TODO: find a better way to handle command input */
	mvaddch(Y_SCANIN, X_SCANIN - 1, '$');
	mvgetnstr(Y_SCANIN, X_SCANIN, buf, IOBUF_SIZE - 1);
	this->iobuf = std::string(buf);

	curs_set(FALSE);
	noecho();
	/* echo guard end */
	return retval;
}

enum GEN_ERR ctrl_unit::parseio(void)
{
	enum GEN_ERR retval = E_OK;

	size_t pos = 0;
	std::string toparse = this->iobuf;
	std::string delim(" ");
	std::string token;
	std::vector<std::string> tokens;

	/* split */
	while ((pos = toparse.find(delim)) != std::string::npos) {
		token = toparse.substr(0, pos);
		tokens.push_back(token);
		toparse.erase(0, pos + delim.length());
	}
	tokens.push_back(toparse);

	/* i don't like this, but whatever */
	if (tokens.size() <= 1) {
		this->iobuf = std::string("err: too few arguments");
		retval = E_ARG;
		return retval;
	} else if (tokens.size() > 3) {
		this->iobuf = std::string("err: too many arguments");
		retval = E_ARG;
		return retval;
	} else if (tokens.size() == 2) {
		if (tokens[0] == std::string("j") || tokens[0] == std::string("jump")) {
			this->set_argaddr_from_str(tokens[1]);
			this->cmd_jumptomem();
		} else if (tokens[0] == std::string("set-pc")) {
			this->set_argaddr_from_str(tokens[1]);
			this->cmd_setpc();
		} else if (tokens[0] == std::string("exe")) {
			this->set_argdata_from_str(tokens[1]);
			this->cmd_exenticks();
		} else if (tokens[0] == std::string("add-b")) {
			this->set_argaddr_from_str(tokens[1]);
			this->cmd_addbreak();
		} else if (tokens[0] == std::string("del-b")) {
			this->set_argaddr_from_str(tokens[1]);
			this->cmd_delbreak();
		}
	} else if (tokens.size() == 3) {
		if (tokens[0] == std::string("poke")) {
			this->set_argaddr_from_str(tokens[1]);
			this->set_argdata_from_str(tokens[2]);
			this->cmd_pokemem();
		}
	}
	return retval;
}

void ctrl_unit::draw(void)
{
	/* draw instruction at current $pc position : REDO */
	uint32_t ypos = mem->rom_ptr - mem->get_rom_beginp();
	if (ypos <= VIEW_MEM_RANGE)
		instr.draw(ypos, X_INSTR);

	/* draw previous cmd arguments */
	mvprintw(Y_ARG_TITLE, X_ARGS, "CMD ARGS:");
	mvprintw(Y_ARG_ADDR, X_ARGS, "a: 0x%04x", this->arg_addr);
	mvprintw(Y_ARG_DATA, X_ARGS, "d: 0x%04x", this->arg_data);

	/* draw scan line area */
	this->iobuf.resize(IOBUF_SIZE);
	mvprintw(Y_SCANIN - 1, X_SCANIN, "%s", this->iobuf.c_str());

	/* draw breakpoints : FIX -> something might break? */
	for (auto const& it : this->bpoints) {
		ypos = it - mem->get_rom_beginp() + 1;
		if (ypos <= VIEW_MEM_RANGE)
			mvprintw(ypos, X_ROM - 3, "[*]");
	}
	return;
}
/* control unit interface END */