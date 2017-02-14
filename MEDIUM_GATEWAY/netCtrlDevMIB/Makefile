#CC=/opt/MPC83xx/bin/powerpc-e300c3-linux-gcc
#CC=powerpc-e300c3-linux-gcc
CC=gcc

EXTERN_INC = ../include/
#==================OSA===========================
OSA_INC = ./osa/inc/
OSA_SRC_PATH = ./osa/src/
OSA_SRCS = ./osa/src/osa_mutex.c ./osa/src/osa_network.c ./osa/src/osa_thr.c ./osa/src/osa_timer.c
OSA_OBJS = $(OSA_SRCS:.c=.o)

#==================Public===========================
PUBLIC_SRCS = public.c paraMng.c
PUBLIC_OBJS = $(PUBLIC_SRCS:.c=.o)

#==================ZhenRuFenJiTab===================
ZRFJ_SRCS = zhenRFenjTableAccess.c zhenRFenjTable.c
ZRFJ_OBJS = $(ZRFJ_SRCS:.c=.o)


#==================main=============================
MAIN_SRCS = main.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)



TARGETS = netCtrlDevMIB.so

BUILDLIBS=`net-snmp-config --libs`
BUILDAGENTLIBS=`net-snmp-config --agent-libs`
NET-SMP-INSTALL-DIR=${snmp_target}

NET-SNMP-HEADFILES="${NET-SMP-INSTALL-DIR}/include"
NET-SNMP-LIBS="${NET-SMP-INSTALL-DIR}/lib"
#CROSS_LIBS=/opt/MPC83xx/powerpc-e300c3-linux/lib/

# shared library flags (assumes gcc)
CFLAGS= -fPIC -I$(NET-SNMP-HEADFILES) -I$(OSA_INC) -I$(EXTERN_INC)
DLFLAGS=-g -fPIC -shared

all:$(TARGETS)

$(TARGETS): $(OSA_OBJS) $(PUBLIC_OBJS) $(ZRFJ_OBJS) $(MAIN_OBJS)
	$(CC) $(DLFLAGS) -o $(TARGETS)  $(OSA_OBJS) $(PUBLIC_OBJS) $(ZRFJ_OBJS)  $(MAIN_OBJS)
	
$(OSA_OBJS):$(OSA_SRCS)
	$(CC) $(CFLAGS) -c -o $@ $(OSA_SRC_PATH)$(*F).c 	
	
$(PUBLIC_OBJS):$(PUBLIC_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 		
		
$(ZRFJ_OBJS):$(ZRFJ_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c  
	
$(MAIN_OBJS):$(MAIN_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 

clean:
		rm *.o $(TARGETS) $(OSA_OBJS) -f

	

