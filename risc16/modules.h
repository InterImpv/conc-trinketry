#include "gen-err.h"

#include <cstdint>
#include <string>
#include <array>

#define N_OF_REGS	8
#define MEM_CAPACITY	65536
#define VIEW_MEM_RANGE	22

#define ROM_START	0x0000
#define ROM_END		0x1fff
#define RAM_START	0x2000
#define RAM_END		0xffff

#define STDOUT_START	0x2000
#define STDOUT_END	0x20ff

class mem_unit {
private:
	/* private members BEGIN */
	uint16_t rom_beginp;
	uint16_t rom_endp;
	uint16_t ram_beginp;
	uint16_t ram_endp;

	std::array<uint16_t, MEM_CAPACITY> mem;
	/* private members END */
	/* private functions BEGIN */
	void __draw_memseg(const uint32_t xpos, const uint32_t ypos,
			   const uint16_t start, const uint16_t end,
			   const uint16_t pos);
	/* private functions END */
public:
	/* public members BEGIN */
	uint16_t rom_ptr;
	uint16_t ram_ptr;
	/* public members END */
	/* public functions BEGIN */
	mem_unit(void) = default;
	~mem_unit(void) = default;

	void reset(void);
	enum GEN_ERR fill(const char *path);

	void inc_rom_ptr(void);
	void dec_rom_ptr(void);
	void inc_ram_ptr(void);
	void dec_ram_ptr(void);
	void set_rom_beginp(const uint16_t addr);
	void set_rom_endp(const uint16_t addr);
	void set_ram_beginp(const uint16_t addr);
	void set_ram_endp(const uint16_t addr);
	const uint16_t get_rom_beginp(void);
	const uint16_t get_rom_endp(void);
	const uint16_t get_ram_beginp(void);
	const uint16_t get_ram_endp(void);

	uint16_t read(const uint16_t addr);
	enum GEN_ERR write(const uint16_t addr, const uint16_t data, bool force);

	void draw(void);
	/* public functions END */
};

class reg_unit {
private:
	/* private members BEGIN */
	uint16_t pc;
	std::array<uint16_t, N_OF_REGS> rx;
	/* private members END */
public:
	reg_unit(void) = default;
	~reg_unit(void) = default;

	void reset(void);

	void inc_pc(void);
	void set_pc(const uint16_t addr);
	const uint16_t get_pc(void);

	uint16_t read(const uint16_t reg);
	void write(const uint16_t reg, const uint16_t data);

	void draw(void);
};