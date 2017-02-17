/**
*Desc: 	main file of sub-agent of net-snmp
*       	As select/recvfrom() always abnoramally hangup in a thread created by 
*		pthread_create in the mod of dynamically load module for snmpd, which
*		using "dlmod $(modName) $(mode.so)". So we have to using another
*		mechanism that net-snmp support to expend snmp-agent, sub_agent.
*		During test, in sub_agent mode, select & recvfrom works in the thread that 
*		pthread_create(). So this is the one we want.
*		the main.c func all get from url: http://www.net-snmp.org/wiki/index.php/TUT:Writing_a_Subagent
*		example-demo.c. I changed nothing in main file except using "init_netCtrlDevMIB()" to replace  init_nstAgentSubagentObject(); 
*		by Andy-wei.hou 2017.02.16
*Author: Andy-wei.hou
*Log:	created by Andy-wei.hou 2017.02.16
*
**/

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <signal.h>

#include "mediaGW.h"
#include "conferenceTable.h"
#include "zhenRFenjTable.h"
#include "paraMng.h"
#include "osa.h"

static int keep_running;

void init_netCtrlDevMIB(){
	osa_init_timer(20);

	init_mediaGW();
	init_zhenRFenjTable();	
	init_conferenceTable();
	
	/*Start thread of Rcving pdu from board*/
	startAgentRcvPduThr();

}

RETSIGTYPE
stop_server(int a) {
    keep_running = 0;
}

int
main (int argc, char **argv) {
  int agentx_subagent=1; /* change this if you want to be a SNMP master agent */
  int background = 0; /* change this if you want to run in the background */
  int syslog = 0; /* change this if you want to use syslog */
  OSA_DBG_MSGX(" ");

  /* print log errors to syslog or stderr */
  if (syslog)
    snmp_enable_calllog();
  else
    snmp_enable_stderrlog();

  /* we're an agentx subagent? */
  if (agentx_subagent) {
    /* make us a agentx client. */
    netsnmp_ds_set_boolean(NETSNMP_DS_APPLICATION_ID, NETSNMP_DS_AGENT_ROLE, 1);
  }

  /* run in background, if requested */
  if (background && netsnmp_daemonize(1, !syslog))
      exit(1);

  /* initialize tcpip, if necessary */
  SOCK_STARTUP;

  /* initialize the agent library */
  init_agent("mngSubAgent");

  /* initialize mib code here */

  /* mib code: init_nstAgentSubagentObject from nstAgentSubagentObject.C */
  init_netCtrlDevMIB();

  /* initialize vacm/usm access control  */
  if (!agentx_subagent) {
      init_vacm_vars();
      init_usmUser();
  }

  /* mngSubAgent will be used to read mngSubAgent.conf files. */
  init_snmp("mngSubAgent");

  /* If we're going to be a snmp master agent, initial the ports */
  if (!agentx_subagent)
    init_master_agent();  /* open the port to listen on (defaults to udp:161) */

  /* In case we recevie a request to stop (kill -TERM or kill -INT) */
  keep_running = 1;
  signal(SIGTERM, stop_server);
  signal(SIGINT, stop_server);

  snmp_log(LOG_INFO,"mngSubAgent is up and running.\n");

  /* your main loop here... */
  while(keep_running) {
    /* if you use select(), see snmp_select_info() in snmp_api(3) */
    /*     --- OR ---  */
    agent_check_and_process(1); /* 0 == don't block */
  }

  /* at shutdown time */
  snmp_shutdown("mngSubAgent");
  SOCK_CLEANUP;

  return 0;
}

