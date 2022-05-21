#include "modules.h"

#include <stddef.h>

#define IOBUF_SIZE 33
#define IOBUF_AMOUNT 4

typedef struct __cmd {
	uint16_t addr;
	uint16_t data;

	char iobuf[IOBUF_SIZE];
	char **cmds;
	size_t cmdc;

	reg_unit *regs;
	mem_unit *mem;
	instr_t *instr;
} cmd_t;

char **strsplit(char *str, const char *delim, size_t *size);

cmd_t *cmd_create(void);
enum GEN_ERR cmd_free(cmd_t *cmd);

enum GEN_ERR cmd_init(cmd_t *cmd, reg_unit *regs, mem_unit *mem, instr_t *instr);
enum GEN_ERR cmd_getline(cmd_t *cmd);
enum GEN_ERR cmd_parseline(cmd_t *cmd);
enum GEN_ERR draw_cmdiobuf(cmd_t *cmd);

void cmd_fetchdecode(cmd_t *cmd);
void cmd_execute(cmd_t *cmd);
