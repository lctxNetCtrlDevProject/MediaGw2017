#ifndef _MGW_COMAND_H_
#define	_MGW_COMAND_H_


#if defined(__cpluscplus) || defined(c_plusplsu)
extern "C"{
#endif

#ifndef NULL
#define NULL	0
#endif

#ifndef U8
#define U8		unsigned char
#endif

#ifndef U16
#define U16		unsigned short
#endif

#ifndef U32
#define U32		unsigned long
#endif

#define	CFG_CBSIZE		64
#define CFG_MAXARGS		4

struct cmd_tbl_s {
	char	*name;		/* Command Name			*/
	int		maxargs;	/* maximum number of arguments	*/
	int		repeatable;	/* autorepeat allowed?		*/
						/* Implementation function	*/
	int		(*cmd)(struct cmd_tbl_s *, int, char *[]);
	char		*usage;		/* Usage message	(short)	*/
};

typedef struct cmd_tbl_s	cmd_tbl_t;

/*
 * Monitor Command
 *
 * All commands use a common argument format:
 *
 * void function (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
 */

typedef	void 	command_t (cmd_tbl_t *, int, int, char *[]);


#define MGW_CMD(name,maxargs,rep,cmd,usage) \
		{#name, maxargs, rep, cmd, usage}


int run_command (const char *cmd, int flag);
cmd_tbl_t *find_cmd (const char *cmd);
int parse_line (char *line, char *argv[]);
extern int do_loopback_test(cmd_tbl_t * cmdtp, int argc, char *argv[]);


#if defined(__cpluscplus) || defined(c_plusplsu)
}
#endif


#endif

