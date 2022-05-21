#include "cmdi.h"
#include "winpos.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <argp.h>

const char *SH_NAME = "./shell";
const char *ERR_ARG = "err: unknown argument";
const char *ERR_ARN = "err: invalid argument count";
const char *ERR_ARI = "err: invalid argument";
const char *ERR_CMD = "err: invalid command";

/* move to utils begin */
static uint16_t clamp_ui16(uint16_t val, uint16_t min, uint16_t max)
{
	const uint32_t ret = (val < min) ? min : val;
	return (ret > max) ? max : ret;
}

/* splits input string to an allocated array of strings
 * the resulting array must be freed to avoid memory leaks */
char **strsplit(char *str, const char *delim, size_t *ssize)
{
	size_t size = 0;
	char **store = NULL;

	char *ptr = strtok(str, delim);
	while (ptr != NULL) {
		store = realloc(store, (size + 1) * sizeof(**store));
		store[size] = ptr;
		ptr = strtok(NULL, delim);
		size++;
	}
	*ssize = size;
	return store;
}
/* move to utils end */

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

/* set inner buffers to '\0' */
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

enum GEN_ERR cmd_init(cmd_t *cmd, reg_unit *regs, mem_unit *mem, instr_t *instr)
{
    if (NULL == cmd || NULL == regs || NULL == mem || NULL == instr)
		return E_ARG;

	cmd->addr = 0;
	cmd->data = 0;
	cmd->regs = regs;
	cmd->mem = mem;
	cmd->instr = instr;

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
	if (cmd->addr >= RAM_START && cmd->addr <= RAM_END) {	// !
		uint16_t addr_end = clamp_ui16(cmd->addr, RAM_START + VIEW_MEM_RANGE, RAM_END);
		uint16_t addr_begin = addr_end - VIEW_MEM_RANGE;

		cmd->mem->ram_endp = addr_end;
		cmd->mem->ram_beginp = addr_begin;
	}
	return;
}

static void cmd_pokemem(cmd_t *cmd)
{
	if (cmd->addr >= ROM_START && cmd->addr < ROM_END) {
		cmd->mem->mem[cmd->addr] = cmd->data;
	}
	if (cmd->addr >= RAM_START && cmd->addr <= RAM_END) {	// !
		cmd->mem->mem[cmd->addr] = cmd->data;
	}
	return;
}

static void cmd_setpc(cmd_t *cmd)
{
	set_pc(cmd->regs, cmd->addr);
	return;
}

static void cmd_exenticks(cmd_t *cmd)
{
	while (cmd->data-- != 0) {
		cmd_fetchdecode(cmd);
		cmd_execute(cmd);
	}
	return;
}
/* unsafe functions end */

static void cmd_puterr(cmd_t *cmd, const char *msg)
{
	strncpy(cmd->iobuf, msg, IOBUF_SIZE);
	return;
}

static void cmd_setarg(cmd_t *cmd, const char *addr, const char *data)
{
	if (NULL != addr) {
		cmd->addr = strtol(addr, NULL, 16);
	}
	if (NULL != data) {
		cmd->data = strtol(data, NULL, 16);
	}
}

void cmd_fetchdecode(cmd_t *cmd)
{
	instr_decode(cmd->instr, instr_fetch(cmd->mem, cmd->regs));
}

void cmd_execute(cmd_t *cmd)
{
	instr_exec(cmd->instr, cmd->regs, cmd->mem);
}

static error_t cmd_parseopt(int key, char *arg, struct argp_state *state)
{
    cmd_t *cmd = state->input;

    switch (key)
    {
    case 'j':	// jump
		if (cmd->cmdc != 3) {
			cmd_puterr(cmd, ERR_ARN);
			break;
		}
		// i don't care about errors right now
		cmd_setarg(cmd, arg, NULL);
		cmd_jmptomem(cmd);
        break;

    case 'p':	// poke
		size_t size_arg = strlen(arg);
		char cpybuf[IOBUF_SIZE];
		if (cmd->cmdc != 3) {
			cmd_puterr(cmd, ERR_ARN);
			break;
		}
		strncpy(cpybuf, arg, size_arg);
		char **args = strsplit(cpybuf, "=", &size_arg);
		if (args == NULL || size_arg != 2) {
			cmd_puterr(cmd, ERR_ARI);
			break;
		}
		// i don't care about errors right now
		cmd_setarg(cmd, args[0], args[1]);
		cmd_pokemem(cmd);
		free(args);
        break;

    case 'c':	// set pc
		if (cmd->cmdc != 3) {
			cmd_puterr(cmd, ERR_ARN);
			break;
		}
		// i don't care about errors right now
		cmd_setarg(cmd, arg, NULL);
		cmd_setpc(cmd);
        break;

    case 's':	// execute n ticks
		if (cmd->cmdc != 3) {
			cmd_puterr(cmd, ERR_ARN);
			break;
		}
		// i don't care about errors right now
		cmd_setarg(cmd, NULL, arg);
		cmd_exenticks(cmd);
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
	/* TODO: find a better way to handle command input */
	mvaddch(Y_SCANIN, X_SCANIN - 1, '$');
	mvgetnstr(Y_SCANIN, X_SCANIN, cmd->iobuf, IOBUF_SIZE - 1);

	curs_set(FALSE);
	noecho();
	/* echo guard end */
    return E_OK;
}

enum GEN_ERR cmd_parseline(cmd_t *cmd)
{
	char cpybuf[IOBUF_SIZE];
	const char *delim = " ";

	/* specific split */
	strncpy(cpybuf, cmd->iobuf, IOBUF_SIZE - 1);
	char *ptr = strtok(cpybuf, delim);
	for (uint32_t i = 1; ptr != NULL && i < IOBUF_AMOUNT; i++) {
		strncpy(cmd->cmds[i], ptr, IOBUF_SIZE - 1);
		ptr = strtok(NULL, delim);
		cmd->cmdc++;
	}

	struct argp_option options[] = {
		/* move to specific address */
		{ "jump", 'j', "ADDR", 0, "0" },
		/* overwrite value in memory */
		{ "poke", 'p', "ADDR DATA", 0, "1" },
		/* set program counter */
		{ "pcset", 'c', "ADDR", 0, "2" },
		/* execute N ticks */
		{ "exes", 's', "TICKS", 0, "3" },
		/* execute until pc != ADDR */
		//{ "exet", 0, "ADDR", 0, "4" },
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
	/* DELETE LATER begin */
	mvprintw(5, 60, "a: 0x%04x", cmd->addr);
	mvprintw(6, 60, "d: 0x%04x", cmd->data);
	/* DELETE LATER end */

    return E_OK;
}
