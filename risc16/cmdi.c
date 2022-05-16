#include "cmdi.h"
#include "winpos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <argp.h>

const char *SH_NAME = "./shell";
const char *ERR_ARG = "err: unknown argument";
const char *ERR_CMD = "err: invalid command";

static uint16_t clamp_ui16(uint16_t val, uint16_t min, uint16_t max)
{
	const uint32_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

cmd_t *cmd_create(void)
{
	cmd_t *cmd = NULL;
	cmd = malloc(sizeof(*cmd));
	if (NULL == cmd)
		return cmd;

	cmd->cmds = malloc(IOBUF_AMOUNT * sizeof(**cmd->cmds));
	if (NULL == cmd->cmds)
		return cmd;

	for (uint32_t i = 0; i < IOBUF_AMOUNT; i++)
		cmd->cmds[i] = malloc(IOBUF_SIZE * sizeof(*cmd->cmds));

	return cmd;
}

enum GEN_ERR cmd_free(cmd_t *cmd)
{
    if (NULL == cmd)
		return E_ARG;

	if (NULL != cmd->cmds)
		free(cmd->cmds);

    free(cmd);
    return E_OK;
}

static void cmd_flushbufs(cmd_t *cmd)
{
	if (NULL == cmd)
		return;

	cmd->cmdc = 1;
	memset(cmd->iobuf, '\0', IOBUF_SIZE * sizeof(*cmd->iobuf));
	for (uint32_t i = 1; i < IOBUF_AMOUNT; i++)
		memset(cmd->cmds[i], '\0', IOBUF_SIZE * sizeof(*cmd->cmds[i]));

	return;
}

enum GEN_ERR cmd_init(cmd_t *cmd, reg_unit *regs, mem_unit *mem)
{
    if (NULL == cmd || NULL == regs || NULL == mem)
		return E_ARG;

	cmd->opcode = 0;
	cmd->addr = 0;
	cmd->data = 0;
	cmd->regs = regs;
	cmd->mem = mem;

	cmd_flushbufs(cmd);
	strncpy(cmd->cmds[0], SH_NAME, strlen(SH_NAME));

    return E_OK;
}

/* unsafe functions begin */
static void cmd_jmptomem(cmd_t *cmd)
{
	if (cmd->addr >= ROM_START && cmd->addr < ROM_END) {
		uint16_t addr_end = clamp_ui16(cmd->addr, ROM_START + VIEW_MEM_RANGE, ROM_END);
		uint16_t addr_begin = addr_end - VIEW_MEM_RANGE;

		cmd->mem->rom_endp = addr_end;
		cmd->mem->rom_beginp = addr_begin;
	}
	return;
}
/* unsafe functions end */

static error_t cmd_parseopt(int key, char *arg, struct argp_state *state)
{
    cmd_t *cmd = state->input;

    switch (key)
    {
    case 'j':
		char *endptr;

		errno = 0;
		cmd->addr = strtol(arg, &endptr, 16);
		/* paranoid check */
		if (errno != 0 || endptr == arg || *endptr != '\0') {
			strncpy(cmd->iobuf, ERR_ARG, strlen(ERR_ARG));
			cmd->addr = 0;
			break;
		}
		cmd_jmptomem(cmd);
        break;

    default:
        return ARGP_ERR_UNKNOWN;
        break;
    };
    return E_OK;
}

enum GEN_ERR cmd_getline(cmd_t *cmd)
{
    if (NULL == cmd)
		return E_ARG;

	cmd_flushbufs(cmd);

	/* echo guard begin */
	echo();
	curs_set(TRUE);

	/* input */
	mvaddch(Y_SCANIN, X_SCANIN - 1, '$');
	mvgetnstr(Y_SCANIN, X_SCANIN, cmd->iobuf, IOBUF_SIZE - 1);

	curs_set(FALSE);
	noecho();
	/* echo guard end */

	char cpybuf[IOBUF_SIZE];
	const char *delim = " ";

	/* split input string */
	strncpy(cpybuf, cmd->iobuf, IOBUF_SIZE - 1);
	char *ptr = strtok(cpybuf, delim);
	for (uint32_t i = 1; ptr != NULL && i < IOBUF_AMOUNT; i++) {
		strncpy(cmd->cmds[i], ptr, IOBUF_SIZE - 1);
		ptr = strtok(NULL, delim);
		cmd->cmdc++;
	}

	struct argp_option options[] = {
		{ "jmp", 'j', "ADDR", 0, "0" },
		{ 0 }
	};
	struct argp argp_struct = {
		options, cmd_parseopt, 0, 0
	};
	argp_parse(&argp_struct, cmd->cmdc, cmd->cmds, ARGP_SILENT, 0, cmd);

    return E_OK;
}

enum GEN_ERR draw_cmdiobuf(cmd_t *cmd)
{
    if (NULL == cmd)
		return E_ARG;

	mvprintw(Y_SCANIN - 1, X_SCANIN, "%s", cmd->iobuf);
	mvprintw(4, 60, "o: 0x%04x", cmd->opcode);
	mvprintw(5, 60, "a: 0x%04x", cmd->addr);
	mvprintw(6, 60, "d: 0x%04x", cmd->data);

    return E_OK;
}
