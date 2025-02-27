/*********************************************************************

    debugcon.c

    Debugger console engine.

    Copyright (c) 1996-2006, Nicola Salmoria and the MAME Team.
    Visit http://mamedev.org for licensing and usage restrictions.

*********************************************************************/

#include "driver.h"
#include "debugcon.h"
#include "debugcpu.h"
#include "debughlp.h"
#include "debugvw.h"
#include "textbuf.h"
#include <stdarg.h>
#include <ctype.h>



/***************************************************************************
    CONSTANTS
***************************************************************************/

#define CONSOLE_BUF_SIZE	(1024 * 1024)
#define CONSOLE_MAX_LINES	(CONSOLE_BUF_SIZE / 20)

#define ERRORLOG_BUF_SIZE	(1024 * 1024)
#define ERRORLOG_MAX_LINES	(ERRORLOG_BUF_SIZE / 20)



/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _debug_command debug_command;
struct _debug_command
{
	debug_command *	next;
	char			command[32];
	const char *	params;
	const char *	help;
	void			(*handler)(int ref, int params, const char **param);
	void			(*handler_ex)(int ref);
	UINT32			flags;
	int				ref;
	int				minparams;
	int				maxparams;
};



/***************************************************************************
    LOCAL VARIABLES
***************************************************************************/

static text_buffer *console_textbuf;
static text_buffer *errorlog_textbuf;

static debug_command *commandlist;



/***************************************************************************

    Initialization and tear down

***************************************************************************/

/*-------------------------------------------------
    debug_console_init - initializes the console
    system
-------------------------------------------------*/

void debug_console_init(running_machine *machine)
{
	/* allocate text buffers */
	console_textbuf = text_buffer_alloc(CONSOLE_BUF_SIZE, CONSOLE_MAX_LINES);
	if (!console_textbuf)
		return;

	errorlog_textbuf = text_buffer_alloc(ERRORLOG_BUF_SIZE, ERRORLOG_MAX_LINES);
	if (!errorlog_textbuf)
		return;

	/* print the opening lines */
	debug_console_printf("MAME new debugger version %s\n", build_version);
	debug_console_printf("Currently targeting %s (%s)\n", Machine->gamedrv->name, Machine->gamedrv->description);

	/* request callback upon exiting */
	add_exit_callback(machine, debug_console_exit);
}


/*-------------------------------------------------
    debug_console_exit - frees the console
    system
-------------------------------------------------*/

void debug_console_exit(running_machine *machine)
{
	/* free allocated memory */
	if (console_textbuf)
		text_buffer_free(console_textbuf);
	console_textbuf = NULL;

	if (errorlog_textbuf)
		text_buffer_free(errorlog_textbuf);
	errorlog_textbuf = NULL;

	/* free the command list */
	while (commandlist)
	{
		debug_command *temp = commandlist;
		commandlist = commandlist->next;
		free(temp);
	}
}



/***************************************************************************

    Command Handling

***************************************************************************/

/*-------------------------------------------------
    trim_parameter - executes a
    command
-------------------------------------------------*/

static void trim_parameter(char **paramptr, int keep_quotes)
{
	char *param = *paramptr;
	int len = strlen(param);
	int repeat;

	/* loop until all adornments are gone */
	do
	{
		repeat = 0;

		/* check for begin/end quotes */
		if (len >= 2 && param[0] == '"' && param[len - 1] == '"')
		{
			if (!keep_quotes)
			{
				param[len - 1] = 0;
				param++;
				len -= 2;
			}
		}

		/* check for start/end braces */
		else if (len >= 2 && param[0] == '{' && param[len - 1] == '}')
		{
			param[len - 1] = 0;
			param++;
			len -= 2;
			repeat = 1;
		}

		/* check for leading spaces */
		else if (len >= 1 && param[0] == ' ')
		{
			param++;
			len--;
			repeat = 1;
		}

		/* check for trailing spaces */
		else if (len >= 1 && param[len - 1] == ' ')
		{
			param[len - 1] = 0;
			len--;
			repeat = 1;
		}
	} while (repeat);

	*paramptr = param;
}


/*-------------------------------------------------
    internal_execute_command - executes a
    command
-------------------------------------------------*/

static CMDERR internal_execute_command(int execute, int params, char **param)
{
	debug_command *cmd, *found = NULL;
	int len, i, foundcount = 0;
	char *p, *command;

	/* no params is an error */
	if (params == 0)
		return CMDERR_NONE;

	/* the first parameter has the command and the real first parameter; separate them */
	for (p = param[0]; *p && isspace(*p); p++) { }
	for (command = p; *p && !isspace(*p); p++) { }
	if (*p != 0)
	{
		*p++ = 0;
		for ( ; *p && isspace(*p); p++) { }
		if (*p != 0)
			param[0] = p;
		else
			params = 0;
	}
	else
		params = 0;

	/* search the command list */
	len = strlen(command);
	for (cmd = commandlist; cmd != NULL; cmd = cmd->next)
		if (!strncmp(command, cmd->command, len))
		{
			foundcount++;
			found = cmd;
			if (strlen(cmd->command) == len)
			{
				foundcount = 1;
				break;
			}
		}

	/* error if not found */
	if (!found)
		return MAKE_CMDERR_UNKNOWN_COMMAND(0);
	if (foundcount > 1)
		return MAKE_CMDERR_AMBIGUOUS_COMMAND(0);

	/* NULL-terminate and trim space around all the parameters */
	for (i = 1; i < params; i++)
		*param[i]++ = 0;

	/* now go back and trim quotes and braces and any spaces they reveal*/
	for (i = 0; i < params; i++)
		trim_parameter(&param[i], found->flags & CMDFLAG_KEEP_QUOTES);

	/* see if we have the right number of parameters */
	if (params < found->minparams)
		return MAKE_CMDERR_NOT_ENOUGH_PARAMS(0);
	if (params > found->maxparams)
		return MAKE_CMDERR_TOO_MANY_PARAMS(0);

	/* execute the handler */
	if (execute)
		(*found->handler)(found->ref, params, (const char **)param);
	return CMDERR_NONE;
}


/*-------------------------------------------------
    internal_parse_command - parses a command
    and either executes or just validates it
-------------------------------------------------*/

static CMDERR internal_parse_command(const char *original_command, int execute)
{
	char command[MAX_COMMAND_LENGTH], parens[MAX_COMMAND_LENGTH];
	char *params[MAX_COMMAND_PARAMS];
	CMDERR result = CMDERR_NONE;
	char *command_start;
	char *p, c = 0;

	/* make a copy of the command */
	strcpy(command, original_command);

	/* loop over all semicolon-separated stuff */
	for (p = command; *p != 0; )
	{
		int paramcount = 0, foundend = FALSE, instring = FALSE, isexpr = FALSE, parendex = 0;

		/* find a semicolon or the end */
		for (params[paramcount++] = p; !foundend; p++)
		{
			c = *p;
			if (instring)
			{
				if (c == '"' && p[-1] != '\\')
					instring = FALSE;
			}
			else
			{
				switch (c)
				{
					case '"':	instring = TRUE; break;
					case '(':
					case '[':
					case '{':	parens[parendex++] = c; break;
					case ')':	if (parendex == 0 || parens[--parendex] != '(') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case ']':	if (parendex == 0 || parens[--parendex] != '[') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case '}':	if (parendex == 0 || parens[--parendex] != '{') return MAKE_CMDERR_UNBALANCED_PARENS(p - command); break;
					case ',':	if (parendex == 0) params[paramcount++] = p; break;
					case ';': 	if (parendex == 0) foundend = TRUE; break;
					case '-':	if (parendex == 0 && paramcount == 1 && p[1] == '-') isexpr = TRUE; *p = c; break;
					case '+':	if (parendex == 0 && paramcount == 1 && p[1] == '+') isexpr = TRUE; *p = c; break;
					case '=':	if (parendex == 0 && paramcount == 1) isexpr = TRUE; *p = c; break;
					case 0:		foundend = TRUE; break;
					default:	*p = tolower(c); break;
				}
			}
		}

		/* check for unbalanced parentheses or quotes */
		if (instring)
			return MAKE_CMDERR_UNBALANCED_QUOTES(p - command);
		if (parendex != 0)
			return MAKE_CMDERR_UNBALANCED_PARENS(p - command);

		/* NULL-terminate if we ended in a semicolon */
		p--;
		if (c == ';') *p++ = 0;

		/* process the command */
		command_start = params[0];

		/* allow for "do" commands */
		if (tolower(command_start[0] == 'd') && tolower(command_start[1] == 'o') && isspace(command_start[2]))
		{
			isexpr = TRUE;
			command_start += 3;
		}

		/* if it smells like an assignment expression, treat it as such */
		if (isexpr && paramcount == 1)
		{
			UINT64 expresult;
			EXPRERR exprerr = expression_evaluate(command_start, debug_get_cpu_info(cpu_getactivecpu())->symtable, &expresult);
			if (exprerr != EXPRERR_NONE)
				return MAKE_CMDERR_EXPRESSION_ERROR(EXPRERR_ERROR_OFFSET(exprerr));
		}
		else
		{
			result = internal_execute_command(execute, paramcount, &params[0]);
			if (result != CMDERR_NONE)
				return MAKE_CMDERR(CMDERR_ERROR_CLASS(result), command_start - command);
		}
	}
	return CMDERR_NONE;
}


/*-------------------------------------------------
    debug_console_execute_command - execute a
    command string
-------------------------------------------------*/

CMDERR debug_console_execute_command(const char *command, int echo)
{
	CMDERR result;

	/* echo if requested */
	if (echo)
		debug_console_printf(">%s\n", command);

	/* parse and execute */
	result = internal_parse_command(command, TRUE);

	/* display errors */
	if (result != CMDERR_NONE)
	{
		if (!echo)
			debug_console_printf(">%s\n", command);
		debug_console_printf(" %*s^\n", CMDERR_ERROR_OFFSET(result), "");
		debug_console_printf("%s\n", debug_cmderr_to_string(result));
	}

	/* update all views */
	if (echo)
	{
		debug_view_update_all();
		debug_refresh_display();
	}
	return result;
}


/*-------------------------------------------------
    debug_console_validate_command - validate a
    command string
-------------------------------------------------*/

CMDERR debug_console_validate_command(const char *command)
{
	return internal_parse_command(command, FALSE);
}


/*-------------------------------------------------
    debug_console_register_command - register a
    command handler
-------------------------------------------------*/

void debug_console_register_command(const char *command, UINT32 flags, int ref, int minparams, int maxparams, void (*handler)(int ref, int params, const char **param))
{
	debug_command *cmd = malloc(sizeof(*cmd));
	assert_always(cmd != NULL, "Out of memory registering new command with the debugger!");
	memset(cmd, 0, sizeof(*cmd));

	/* fill in the command */
	strcpy(cmd->command, command);
	cmd->flags = flags;
	cmd->ref = ref;
	cmd->minparams = minparams;
	cmd->maxparams = maxparams;
	cmd->handler = handler;

	/* link it */
	cmd->next = commandlist;
	commandlist = cmd;
}



/***************************************************************************

    Error Handling

***************************************************************************/

/*-------------------------------------------------
    debug_cmderr_to_string - return a friendly
    string for a given command error
-------------------------------------------------*/

const char *debug_cmderr_to_string(CMDERR error)
{
	switch (CMDERR_ERROR_CLASS(error))
	{
		case CMDERR_UNKNOWN_COMMAND:		return "unknown command";
		case CMDERR_AMBIGUOUS_COMMAND:		return "ambiguous command";
		case CMDERR_UNBALANCED_PARENS:		return "unbalanced parentheses";
		case CMDERR_UNBALANCED_QUOTES:		return "unbalanced quotes";
		case CMDERR_NOT_ENOUGH_PARAMS:		return "not enough parameters for command";
		case CMDERR_TOO_MANY_PARAMS:		return "too many parameters for command";
		case CMDERR_EXPRESSION_ERROR:		return "error in assignment expression";
		default:							return "unknown error";
	}
}



/***************************************************************************

    Console Management

***************************************************************************/

/*-------------------------------------------------
    debug_console_printf - printfs the given
    arguments using the format to the debug
    console
-------------------------------------------------*/

void CLIB_DECL debug_console_printf(const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vsprintf(giant_string_buffer, format, arg);
	va_end(arg);

	text_buffer_print(console_textbuf, giant_string_buffer);

	/* force an update of any console views */
	debug_view_update_type(DVT_CONSOLE);
}


/*-------------------------------------------------
    debug_console_printf_wrap - printfs the given
    arguments using the format to the debug
    console
-------------------------------------------------*/

void CLIB_DECL debug_console_printf_wrap(int wrapcol, const char *format, ...)
{
	va_list arg;

	va_start(arg, format);
	vsprintf(giant_string_buffer, format, arg);
	va_end(arg);

	text_buffer_print_wrap(console_textbuf, giant_string_buffer, wrapcol);

	/* force an update of any console views */
	debug_view_update_type(DVT_CONSOLE);
}


/*-------------------------------------------------
    debug_console_get_textbuf - return a pointer
    to the console text buffer
-------------------------------------------------*/

text_buffer *debug_console_get_textbuf(void)
{
	return console_textbuf;
}


/*-------------------------------------------------
    debug_errorlog_write_line - writes a line to
    the errorlog ring buffer
-------------------------------------------------*/

void debug_errorlog_write_line(running_machine *machine, const char *line)
{
	if (errorlog_textbuf)
		text_buffer_print(errorlog_textbuf, line);

	/* force an update of any log views */
	debug_view_update_type(DVT_LOG);
}


/*-------------------------------------------------
    debug_errorlog_get_textbuf - return a pointer
    to the errorlog text buffer
-------------------------------------------------*/

text_buffer *debug_errorlog_get_textbuf(void)
{
	return errorlog_textbuf;
}

