#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "zhenRFenjTable.h"
#include "paraMng.h"
#include "osa.h"


void init_netCtrlDevMIB(){
	osa_init_timer(20);
	/*Start thread of Rcving pdu from board*/
	startAgentRcvPduThr();

	init_zhenRFenjTable();

}
