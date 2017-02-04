#include "PUBLIC.h"

extern const char version_string[];

int do_version (cmd_tbl_t *cmdtp, int argc, char *argv[]);
int do_help (cmd_tbl_t * cmdtp, int argc, char *argv[]);
int do_sw_action(cmd_tbl_t * cmdtp, int argc, char *argv[]);
int do_terminate(cmd_tbl_t * cmdtp, int argc, char *argv[]);
int do_find_node(cmd_tbl_t * cmdtp, int argc, char *argv[]);
extern int do_loopback_test(cmd_tbl_t * cmdtp, int argc, char *argv[]);

static cmd_tbl_t mgw_full_cmd[] = {
MGW_CMD(version,1,1,do_version,"version - print monitor version\n"),
MGW_CMD(iodump,1,1,do_io_dump,"iodump - dump io sched pro\n"),
MGW_CMD(dump,2,1,do_buf_dump,"dump open/close - open or close buf dump\n"),
MGW_CMD(rt,1,1,do_show_route,"rt - show route\n"),
MGW_CMD(sendtone,2,1,do_sendtone,"sendtone p1 p2 - send tone to port\n"),
MGW_CMD(senddtmf,2,1,do_senddtmf,"senddtmf port xxxxx - send dtmf xxxx to port\n"),
MGW_CMD(help,1,1,do_help,"help - list all command\n"),
//MGW_CMD(addnode,3,1,do_add_js_node,"addnode - addnode xxxxxxxx xxx.xxx.xxx.xxx\n"),
//MGW_CMD(subnode,2,1,do_sub_js_node,"subnode - subnode xxx.xxx.xxx.xxx\n"),
//MGW_CMD(addroute,2,1,do_insert_js_rt,"addroute - addroute xxxxxxxx xxxxxxxx cost\n"),
MGW_CMD(broadcast,1,1,do_broadcast_test,"broadcast - broadcast 192.168.2.255\n"),
MGW_CMD(iprt,2,1,do_submit_ip_route,"iprt - iprt xxx.xxx.xxx.xxx xx\n"),
MGW_CMD(iprtdel,2,1,do_submit_del_ip_route,"iprtdel - iprtdel xxx.xxx.xxx.xxx xx\n"),
MGW_CMD(hfrt,1,1,do_query_hf_route,"hfrt - hfrt\n"),
MGW_CMD(trk,2,1,do_trk_action,"trk use/free 0/1 - use or free trk 0/1\n"),
MGW_CMD(listen,2,1,do_listen_slot,"listen xx - listen && speack to slot in AC491\n"),
MGW_CMD(sw,2,1,do_sw_action,"sw xx ac491/st_bus - switch xx to AC491 or ST_BUS\n"),
MGW_CMD(hear,2,1,do_hear_slot,"hear [stop] - hear rtp packet\n"),
//MGW_CMD(nat,1,1,do_nat_atction,"nat [add|del] [port] [ip] - show/add/del nat table\n"),
MGW_CMD(findnode,2,1,do_find_node,"findnode callee - find node by callee num\n"),
MGW_CMD(loopback,2,1,do_loopback_test,"loopback - do loopback test,QINWU hear own voice\n"),
MGW_CMD(exit,1,1,do_terminate,"exit - Terminate Process\n"),
MGW_CMD(end,1,1, NULL, "end - Terminate Process\n"),
};


int do_version (cmd_tbl_t *cmdtp, int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv)||(NULL == cmdtp))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
	DEBUG_OUT("%s\n", version_string);
	return 0;
}

int do_help (cmd_tbl_t * cmdtp, int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv)||(NULL == cmdtp))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	cmd_tbl_t *cmd_index = mgw_full_cmd;
	for(cmd_index = mgw_full_cmd;
		strncmp(cmd_index->name,"end",3);
		cmd_index++)
		DEBUG_OUT("%s",cmd_index->usage);

	return 0;
}

int do_sw_action(cmd_tbl_t * cmdtp, int argc,char * argv [ ])
{
	int ch;

    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	if(argc != 3){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}
	ch = atoi(argv[1]);

	if((ch < 0)||(ch >4)){
		printf("Invalid Parameter!\n");
		return 0;
	}

	if(!strcmp(argv[2],"ac491"))
		sw2ac491(ch);
	else if(!strcmp(argv[2],"st_bus"))
		sw2st_bus(ch);
	else 
		printf("Invalid Parameter!\n");

	return 0;
}

int do_find_node(cmd_tbl_t * cmdtp,int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	if(argc != 2){
		printf("Invalid Parameter!\n");
		printf("usage: %s",cmdtp->usage);
		return 0;
	}
	printf("find node %.8x\n",find_node_by_callee(argv[1]));
	return 0;
}


int do_terminate(cmd_tbl_t * cmdtp, int argc, char *argv[])
{
    if((0 == argc)||(NULL == argv)||(NULL == cmdtp))
    {
        ERR("%s: param error\r\n", __func__);
        return DRV_ERR;
    }
    
	exit(0);
	return 0;
}

/***************************************************************************
 * find command table entry for a command
 */
cmd_tbl_t *find_cmd (const char *cmd)
{
	cmd_tbl_t *cmdtp;
	cmd_tbl_t *cmdtp_temp = mgw_full_cmd;	/*Init value */
	const char *p;
	int len;
	int n_found = 0;
	
#ifdef DEBUG_PARSER
	printf ("find_cmd: %s\n", cmd);
#endif

	/*
	 * Some commands allow length modifiers (like "cp.b");
	 * compare command name only until first dot.
	 */
	len = ((p = strchr(cmd, '.')) == NULL) ? strlen (cmd) : (p - cmd);

	for (cmdtp = mgw_full_cmd;
	     strncmp(cmdtp->name,"end",3);
	     cmdtp++) {
		if (strncmp (cmd, cmdtp->name, len) == 0) {
			if (len == strlen (cmdtp->name))
				return cmdtp;	/* full match */

			cmdtp_temp = cmdtp;	/* abbreviated command ? */
			n_found++;
		}
	}
	if (n_found == 1) {			/* exactly one match */
		return cmdtp_temp;
	}

	return NULL;	/* not found or ambiguous command */
}


/****************************************************************************/

int parse_line (char *line, char *argv[])
{
	int nargs = 0;

#ifdef DEBUG_PARSER
	printf ("parse_line: \"%s\"\n", line);
#endif
	while (nargs < CFG_MAXARGS) {

		/* skip any white space */
		while ((*line == ' ') || (*line == '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		argv[nargs++] = line;	/* begin of argument string	*/

		/* find end of string */
		while (*line && (*line != ' ') && (*line != '\t')) {
			++line;
		}

		if (*line == '\0') {	/* end of line, no more args	*/
			argv[nargs] = NULL;
#ifdef DEBUG_PARSER
		printf ("parse_line: nargs=%d\n", nargs);
#endif
			return (nargs);
		}

		*line++ = '\0';		/* terminate current arg	 */
	}

	printf ("** Too many args (max. %d) **\n", CFG_MAXARGS);

#ifdef DEBUG_PARSER
	printf ("parse_line: nargs=%d\n", nargs);
#endif
	return (nargs);
}

/****************************************************************************/


/****************************************************************************
 * returns:
 *	1  - command executed, repeatable
 *	0  - command executed but not repeatable, interrupted commands are
 *	     always considered not repeatable
 *	-1 - not executed (unrecognized, bootd recursion or too many args)
 *           (If cmd is NULL or "" or longer than CFG_CBSIZE-1 it is
 *           considered unrecognized)
 *
 * WARNING:
 *
 * We must create a temporary copy of the command since the command we get
 * may be the result from getenv(), which returns a pointer directly to
 * the environment data, which may change magicly when the command we run
 * creates or modifies environment variables (like "bootp" does).
 */

int run_command (const char *cmd, int flag)
{
	cmd_tbl_t *cmdtp;
	char cmdbuf[CFG_CBSIZE];	/* working copy of cmd		*/
	char *token;			/* start of token in cmdbuf	*/
	char *sep;			/* end of token (separator) in cmdbuf */
	char finaltoken[CFG_CBSIZE] = {0};
	char *str = cmdbuf;
	char *argv[CFG_MAXARGS + 1];	/* NULL terminated	*/
	int argc, inquotes;
	int repeatable = 1;
	int rc = 0;

	if (!cmd || !*cmd) {
		return -1;	/* empty command */
	}

	//strcpy (cmdbuf, cmd);
	strncpy((char *)cmdbuf, cmd, sizeof(cmdbuf));

	/* Process separators and check for invalid
	 * repeatable commands
	 */
	 
#ifdef DEBUG_PARSER
	printf ("cmdbuf:%s\n", cmdbuf);
#endif


	if ((argc = parse_line (cmdbuf, argv)) == 0) {
		rc = -1;	/* no command at all */
		return rc;
	}
	
	/* Look up command in command table */
	if ((cmdtp = find_cmd(argv[0])) == NULL) {
		printf ("Unknown command '%s' - try 'help'\n", argv[0]);
		rc = -1;	/* give up after bad command */
		return rc;
	}
	
	/* OK - call function to do the command */
	if ((cmdtp->cmd) (cmdtp, argc, argv) != 0) {
		rc = -1;
		return rc;
	}
	
	while (*str) {
		/*
		 * Find separator, or string end
		 * Allow simple escape of ';' by writing "\;"
		 */
		for (inquotes = 0, sep = str; *sep; sep++) {
			if ((*sep=='\'') &&
			    (*(sep-1) != '\\'))
				inquotes=!inquotes;

			if (!inquotes &&
			    (*sep == ';') &&	/* separator		*/
			    ( sep != str) &&	/* past string start	*/
			    (*(sep-1) != '\\'))	/* and NOT escaped	*/
				break;
		}

		/*
		 * Limit the token to data between separators
		 */
		token = str;
		if (*sep) {
			str = sep + 1;	/* start of command for next pass */
			*sep = '\0';
		}
		else
			str = sep;	/* no more commands for next pass */

		/* Extract arguments */
		if ((argc = parse_line (finaltoken, argv)) == 0) {
			rc = -1;	/* no command at all */
			continue;
		}

		/* Look up command in command table */
		if ((cmdtp = find_cmd(argv[0])) == NULL) {
			printf ("Unknown command '%s' - try 'help'\n", argv[0]);
			rc = -1;	/* give up after bad command */
			continue;
		}

		/* found - check max args */
		if (argc > cmdtp->maxargs) {
			printf ("Usage:\n%s\n", cmdtp->usage);
			rc = -1;
			continue;
		}

		/* OK - call function to do the command */
		if ((cmdtp->cmd) (cmdtp, argc, argv) != 0) {
			rc = -1;
		}

		repeatable &= cmdtp->repeatable;
	}

	return rc ? rc : repeatable;
}

/****************************************************************************/

