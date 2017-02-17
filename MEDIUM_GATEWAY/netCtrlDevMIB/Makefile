#CC=/opt/MPC83xx/bin/powerpc-e300c3-linux-gcc
#CC=powerpc-e300c3-linux-gcc

CrossCompile =1 
ifdef CrossCompile
	CC=powerpc-linux-gcc
	export snmp_target=/work/lctx_work/netCtrlDev2017/NET-SNMP/PKG/crossNetSnmp-5.7.3/target
else
CC=gcc
endif

EXTERN_INC = ../include/


#==================OSA===========================
OSA_INC = ./osa/inc/
OSA_SRC_PATH = ./osa/src/
OSA_SRCS = ./osa/src/osa_mutex.c ./osa/src/osa_network.c ./osa/src/osa_thr.c ./osa/src/osa_timer.c
OSA_OBJS = $(OSA_SRCS:.c=.o)

#==================Public===========================
PUBLIC_SRCS = public.c paraMng.c
PUBLIC_OBJS = $(PUBLIC_SRCS:.c=.o)

#==================mgwPara===================
MGW_PARA_SRCS = mgwParaAccess.c zhenRFenjTable.c conferenceTable.c mediaGW.c
MGW_PARA_OBJS = $(MGW_PARA_SRCS:.c=.o)


#==================main=============================
MAIN_SRCS = main.c
MAIN_OBJS = $(MAIN_SRCS:.c=.o)



#TARGETS = netCtrlDevMIB.so
TARGETS = mngSubAgent


NET-SMP-INSTALL-DIR=${snmp_target}

NET-SNMP-HEADFILES="${NET-SMP-INSTALL-DIR}/include"
NET-SNMP-LIBS="${NET-SMP-INSTALL-DIR}/lib/
#CROSS_LIBS=/opt/MPC83xx/powerpc-e300c3-linux/lib/

BUILDLIBS=`net-snmp-config --libs`
ifdef CrossCompile
	BUILDAGENTLIBS=`/work/lctx_work/netCtrlDev2017/NET-SNMP/PKG/crossNetSnmp-5.7.3/target/bin/net-snmp-config --agent-libs`
else
	BUILDAGENTLIBS=`net-snmp-config --agent-libs`
endif
# shared library flags (assumes gcc)
CFLAGS= -fPIC -I$(NET-SNMP-HEADFILES) -I$(OSA_INC) -I$(EXTERN_INC)  
#DLFLAGS=-g -fPIC -shared 

all:$(TARGETS)

$(TARGETS): $(OSA_OBJS) $(PUBLIC_OBJS) $(MGW_PARA_OBJS) $(MAIN_OBJS)
	$(CC) -o $(TARGETS) $(OSA_OBJS) $(PUBLIC_OBJS) $(MGW_PARA_OBJS)  $(MAIN_OBJS) $(BUILDAGENTLIBS) 
	
$(OSA_OBJS):$(OSA_SRCS)
	$(CC) $(CFLAGS) -c -o $@ $(OSA_SRC_PATH)$(*F).c 	
	
$(PUBLIC_OBJS):$(PUBLIC_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 		
		
$(MGW_PARA_OBJS):$(MGW_PARA_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c  
	
$(MAIN_OBJS):$(MAIN_SRCS)
	$(CC) $(CFLAGS) -c -o $@ ./$(*F).c 

clean:
		rm *.o $(TARGETS) $(OSA_OBJS) -f

	

