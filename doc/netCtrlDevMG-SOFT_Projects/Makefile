#CC=/opt/MPC83xx/bin/powerpc-e300c3-linux-gcc
#CC=powerpc-e300c3-linux-gcc
CC=gcc

OBJS2 = netCtrlDevMIB.o
TARGETS = netCtrlDevMIB.so

BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`
NET-SMP-INSTALL-DIR=${snmp_target}

NET-SNMP-HEADFILES="${NET-SMP-INSTALL-DIR}/include"
NET-SNMP-LIBS="${NET-SMP-INSTALL-DIR}/lib"
#CROSS_LIBS=/opt/MPC83xx/powerpc-e300c3-linux/lib/

# shared library flags (assumes gcc)
CFLAGS= -fPIC
DLFLAGS=-g -fPIC -shared
SRCS = netCtrlDevMIB.c

all:
		$(CC) $(CFLAGS) -c -o $(OBJS2) $(SRCS)  -I$(NET-SNMP-HEADFILES) 
		$(CC) $(DLFLAGS) -o $(TARGETS) $(OBJS2) 

clean:
		rm $(OBJS2) $(TARGETS) -f

	

