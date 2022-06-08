#include "modules.h"
#include "winpos.h"

#include <iostream>
#include <fstream>
#include <cctype>
#include <ncurses.h>

static const uint32_t STDOUT_W = 32;
static const uint32_t STDOUT_H = 8;

/* memory unit interface BEGIN */
void mem_unit::reset(void)
{
	this->rom_ptr = 0;
	this->rom_beginp = ROM_START;
	this->rom_endp = ROM_START + VIEW_MEM_RANGE;

	this->ram_ptr = 0;
	this->ram_beginp = RAM_START;
	this->ram_endp = RAM_START + VIEW_MEM_RANGE;

	this->mem.fill(0);
}

enum GEN_ERR mem_unit::fill(const char *path)
{
	enum GEN_ERR retval = E_OK;

	std::string line;
	std::ifstream prog(path);
	if (!prog.is_open()) {
		retval = E_IO;
		std::cerr << "ERR " << retval << ": file \"" << path << "\" not found\n";
		return retval;
	}

	uint16_t addr = 0;
	while (getline(prog, line) && addr < ROM_END) {
		uint32_t data = 0;
		try {
			data = std::stoi(line, nullptr, 16);
		} catch(std::invalid_argument const& ex) {
			retval = E_IO;
			std::cerr << "ERR " << retval << ": invalid data at line " << addr << " {" << line << "}\n";
			return retval;
		} catch(std::out_of_range const& ex) {
			retval = E_RANGE;
			std::cerr << "ERR " << retval << ": data at line " << addr << " {" << line << "} out of range\n";
			return retval;
		}
		if (data > 0xffff) {
			retval = E_RANGE;
			std::cerr << "ERR " << retval << ": data at line " << addr << " {" << line << "} out of range\n";
			return retval;
		}
		this->mem[addr] = data;
		addr++;
	}
	prog.close();
	return retval;
}

void mem_unit::inc_rom_ptr(void)
{
	if (this->rom_endp < ROM_END) {
		this->rom_beginp++;
		this->rom_endp++;
	}
	return;
}

void mem_unit::dec_rom_ptr(void)
{
	if (this->rom_beginp > ROM_START) {
		this->rom_beginp--;
		this->rom_endp--;
	}
	return;
}

void mem_unit::inc_ram_ptr(void)
{
	if (this->ram_endp < RAM_END) {
		this->ram_beginp++;
		this->ram_endp++;
	}
	return;
}

void mem_unit::dec_ram_ptr(void)
{
	if (this->ram_beginp > RAM_START) {
		this->ram_beginp--;
		this->ram_endp--;
	}
	return;
}

void mem_unit::set_rom_beginp(const uint16_t addr)
{
	this->rom_beginp = addr;
}

void mem_unit::set_rom_endp(const uint16_t addr)
{
	this->rom_endp = addr;
}

void mem_unit::set_ram_beginp(const uint16_t addr)
{
	this->ram_beginp = addr;
}

void mem_unit::set_ram_endp(const uint16_t addr)
{
	this->ram_endp = addr;
}

const uint16_t mem_unit::get_rom_beginp(void)
{
	return this->rom_beginp;
}
const uint16_t mem_unit::get_rom_endp(void)
{
	return this->rom_beginp;
}

const uint16_t mem_unit::get_ram_beginp(void)
{
	return this->ram_beginp;
}
const uint16_t mem_unit::get_ram_endp(void)
{
	return this->ram_beginp;
}

// technically unsafe
uint16_t mem_unit::read(const uint16_t addr)
{
	return this->mem[addr];
}

enum GEN_ERR mem_unit::write(const uint16_t addr, const uint16_t data, const bool force)
{
	enum GEN_ERR retval = E_OK;
	if (addr >= ROM_START && addr <= ROM_END) {
		if (force) {
			this->mem[addr] = data;
		} else {
			retval = E_ROMAC;
			return retval;
		}
	}
	if (addr >= RAM_START && addr <= RAM_END) {
		this->mem[addr] = data;
	}
	return retval;
}

void mem_unit::__draw_memseg(const uint32_t ypos, const uint32_t xpos, const uint16_t start, const uint16_t end, const uint16_t pos)
{
	attron(A_STANDOUT);
	mvprintw(ypos, xpos, "@0x%04x-0x%04x", start, end);
	attroff(A_STANDOUT);
	uint32_t addr, offset;
	for (addr = start, offset = 1; addr <= end; addr++, offset++) {
		if (addr == pos)
			attron(A_BOLD);

		if (isprint(this->mem[addr]) && isascii(this->mem[addr])) {
			mvprintw(ypos + offset, xpos, " 0x%04x 0x%04x: %c", addr, this->mem[addr], this->mem[addr]);
		} else {
			mvprintw(ypos + offset, xpos, " 0x%04x 0x%04x:", addr, this->mem[addr]);
		}
		attroff(A_BOLD);
	}
}

static void rectangle(uint32_t y1, uint32_t x1, uint32_t y2, uint32_t x2)
{
	mvhline(y1, x1, ACS_HLINE, x2 - x1);
	mvhline(y2, x1, ACS_HLINE, x2 - x1);
	mvvline(y1, x1, ACS_VLINE, y2 - y1);
	mvvline(y1, x2, ACS_VLINE, y2 - y1);
	mvaddch(y1, x1, ACS_ULCORNER);
	mvaddch(y2, x1, ACS_LLCORNER);
	mvaddch(y1, x2, ACS_URCORNER);
	mvaddch(y2, x2, ACS_LRCORNER);
}

void mem_unit::draw(void)
{
	this->__draw_memseg(Y_ROM, X_ROM, this->rom_beginp, this->rom_endp, this->rom_ptr);
	this->__draw_memseg(Y_RAM, X_RAM, this->ram_beginp, this->ram_endp, this->ram_ptr);
	/* draw stdout */
	attron(A_BOLD);
	rectangle(Y_STDOUT - 1, X_STDOUT - 1, Y_STDOUT + STDOUT_H,
		  X_STDOUT + STDOUT_W);
	attroff(A_BOLD);
	for (uint32_t i = 0; i < STDOUT_H; i++) {
		for (uint32_t j = 0; j < STDOUT_W; j++) {
			uint16_t index = STDOUT_START + (STDOUT_W * i + j);

			if (isprint(this->mem[index]))
				mvaddch(Y_STDOUT + i, X_STDOUT + j, this->mem[index]);
		}
	}
}
/* memory unit interface END */

/* register unit interface BEGIN */
void reg_unit::reset(void)
{
	this->pc = 0;
	this->rx.fill(0);
}

void reg_unit::inc_pc(void)
{
	this->pc++;
}
void reg_unit::set_pc(const uint16_t addr)
{
	this->pc = addr;
}

const uint16_t reg_unit::get_pc(void)
{
	return this->pc;
}

uint16_t reg_unit::read(const uint16_t reg)
{
	if (reg == 0 || reg >= N_OF_REGS)
		return 0;
	else
	 	return this->rx[reg];
}
void reg_unit::write(const uint16_t reg, const uint16_t data)
{
	if (reg == 0 || reg >= N_OF_REGS)
		return;
	else
	 	this->rx[reg] = data;
}

void reg_unit::draw(void)
{
	mvprintw(Y_REGS, X_REGS, "REGISTERS:");
	for (int i = 0; i < N_OF_REGS; i++) {
		mvprintw(Y_REGS + i + 1, X_REGS, "$R%d: 0x%04x", i, this->rx[i]);
	}
	mvprintw(Y_REGS + N_OF_REGS + 1, X_REGS, "$PC: 0x%04x", this->pc);
}
/* register unit interface END */