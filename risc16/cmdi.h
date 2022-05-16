#include "modules.h"

#define IOBUF_SIZE 33
#define IOBUF_AMOUNT 4

enum cntr_sig {
    CMD_JMPMEM,
    CMD_POKE,
    CMD_SETPC,
    CMD_SETBREAK
};

typedef struct __cmd {
	enum cntr_sig opcode;
	uint16_t addr;
	uint16_t data;

	char iobuf[IOBUF_SIZE];
	char **cmds;
	uint32_t cmdc;

	reg_unit *regs;
	mem_unit *mem;
} cmd_t;

cmd_t *cmd_create(void);
enum GEN_ERR cmd_free(cmd_t *cmd);

enum GEN_ERR cmd_init(cmd_t *cmd, reg_unit *regs, mem_unit *mem);
enum GEN_ERR cmd_getline(cmd_t *cmd);
enum GEN_ERR draw_cmdiobuf(cmd_t *cmd);
