#CC=/opt/MPC83xx/bin/powerpc-e300c3-linux-gcc
#CC=powerpc-e300c3-linux-gcc
CC=gcc

#==================Public===========================
PUBLIC_SRCS = public.c
PUBLIC_OBJS = $(PUBLIC_SRCS:.c=.o)

#==================ZhenRuFenJiTab===================
ZRFJ_SRCS = zhenRFenjTableAccess.c zhenRFenjTable.c
ZRFJ_OBJS = $(ZRFJ_SRCS:.c=.o)


#==================main=============================
MAIN_SRCS = netCtrlDevMIB.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)



TARGETS = netCtrlDevMIB.so

BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`
NET-SMP-INSTALL-DIR=${snmp_target}

NET-SNMP-HEADFILES="${NET-SMP-INSTALL-DIR}/include"
NET-SNMP-LIBS="${NET-SMP-INSTALL-DIR}/lib"
#CROSS_LIBS=/opt/MPC83xx/powerpc-e300c3-linux/lib/

# shared library flags (assumes gcc)
CFLAGS= -fPIC -I$(NET-SNMP-HEADFILES)
DLFLAGS=-g -fPIC -shared

all:$(TARGETS)

$(TARGETS): $(PUBLIC_OBJS) $(ZRFJ_OBJS) $(MAIN_OBJS)
	$(CC) $(DLFLAGS) -o $(TARGETS)  $(PUBLIC_OBJS) $(ZRFJ_OBJS)  $(MAIN_OBJS)
	
$(PUBLIC_OBJS):$(PUBLIC_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 		
		
$(ZRFJ_OBJS):$(ZRFJ_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c  
	
$(MAIN_OBJS):$(MAIN_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 

clean:
		rm *.o $(TARGETS) -f

	

