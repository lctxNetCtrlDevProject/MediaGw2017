#===========================================================
# Makefile for Medium_GW
# 
#===========================================================

#TARGETCPU=i386
TARGETCPU=ppc

#===================PROJECT PATH DEFINE=====================
TOP_DIR = .
SRC_PATH = $(TOP_DIR)/src
INCLUDE_PATH = $(TOP_DIR)/include
TARGET_TEMPDIR = $(TOP_DIR)/target
TARGET_PROG = $(TOP_DIR)/$(TARGET)
empty := 
SPACE := $(empty) $(empty)
#===========================================================


#===================target PPC==============================
ifeq ($(TARGETCPU),ppc)

CROSS_COMPILER ?= powerpc-linux-

TARGET=MEDIUM_GATEWAY

ARCH_EXT_CFLAGS ?= -fsigned-char -Wstrict-prototypes -Wmissing-prototypes -Wmissing-declarations

INCLUDES=	-I$(INCLUDE_PATH) -I$(TOP_DIR)/MGW_Drv

CFLAG=		-mpowerpc \
		-msoft-float \
		-D__KERNEL__ \
		-DDEBUG_INFO \
		-Wall -Wno-comment -Wno-strict-prototypes \
		-Wno-trigraphs \
		-Wno-format \
		-Wno-implicit-function-declaration \
		-fno-builtin \
		-fno-strict-aliasing -fno-common \
		$(INCLUDES) \
		$(ARCH_EXT_CFLAGS) \
		-O2 -static -Wall -pipe

		
LNKFLAG=	-lpthread \
		-lresolv \
		-lcrypt \
		-avoid-version \
		-export-dynamic
		
EXTERN_OBJ=
endif
#============================================================

#===================target x86===============================
ifeq ($(TARGETCPU),i386)
TARGET=MEDIUM_GATEWAY_x86
CROSS_COMPILER =
INCLUDES=	-I$(INCLUDE_PATH)
CFLAG=	-Wall -Wno-comment -Wno-strict-prototypes \
		-Wno-trigraphs \
		-Wno-format \
		-Wno-implicit-function-declaration \
		-fno-builtin \
		-fno-strict-aliasing -fno-common \
		$(INCLUDES) \
		-O2 -static -Wall -DDEBUG_INFO -msoft-float -g
		
LNKFLAG=	-lpthread \
		-lresolv \
		-lcrypt \
		-avoid-version \
		-export-dynamic

EXTERN_OBJ=

endif
#===========================================================


#======================COMPILE DEFINE=======================
PRE_CC=		
CC=		$(CROSS_COMPILER)gcc
CXX=	$(CROSS_COMPILER)g++
LD=		$(CROSS_COMPILER)ld
AR=		$(CROSS_COMPILER)ar crv

#========================BASIC MODULE=======================
BASIC_CSRCS = logger.c \
		poll.c \
		sched.c \
		io.c \
		threadstorage.c \
		utils.c		\
		gpio_api.c           \
		uart.c        \
		zjct_proc.c   \
		pcm_comm.c

BASIC_COBJS = $(BASIC_CSRCS:%.c=%.o)

BASIC_PATHCSRCS := $(BASIC_CSRCS:%=$(SRC_PATH)/%)
BASIC_PATHCOBJS := $(BASIC_COBJS:%=$(TARGET_TEMPDIR)/%)

#=======================MAIN MODULE=========================
MAIN_PATH=	$(SRC_PATH)

MAINCSRCS=	MGW_main.c \
	MGW_call_control.c \
	MGW_config.c \
	MGW_lan_bus_control.c \
	MGW_protocol_parse.c \
	MGW_vlan_bus_control.c \
	MGW_time.c \
	MGW_common.c \
	MGW_command.c \
	MGW_rcb.c \
	MGWparaInject.c \
	paraParse716.c \
	paraParse834.c \
	phone.c  \
	adapter.c

MAIN_COBJS := $(MAINCSRCS:%.c=%.o)

MAIN_PATHCSRCS := $(MAIN_CSRCS:%=$(MAIN_PATH)/%)
MAIN_PATHCOBJS := $(MAIN_COBJS:%=$(TARGET_TEMPDIR)/%)
#===========================================================

#=========================AC491=============================
AC491_PATH=	$(TOP_DIR)/ac491
AC491CSRCS=	AC49xDrv_Api.c \
			AC49xDrv_UserDefinedFunctions.c

AC491_COBJS := $(AC491CSRCS:%.c=%.o)

AC491_PATHCSRCS :=$(AC491_CSRCS:$(AC491_PATH)/%)
AC491_PATHCOBJS :=$(AC491_COBJS:%=$(TARGET_TEMPDIR)/%)

INCLUDES += $(SPACE)
INCLUDES += -I$(AC491_PATH)
#===========================================================

#=========================paraInjectAgent=============================
paraInjectAgent_PATH=	$(TOP_DIR)/paraInjectAgent
paraInjectAgentCSRCS=	paraInjectAgent.c
		

paraInjectAgent_COBJS := $(paraInjectAgentCSRCS:%.c=%.o)

paraInjectAgent_PATHCSRCS :=$(paraInjectAgent_CSRCS:$(paraInjectAgent_PATH)/%)
paraInjectAgent_PATHCOBJS :=$(paraInjectAgent_COBJS:%=$(TARGET_TEMPDIR)/%)

INCLUDES += $(SPACE)
INCLUDES += -I$(paraInjectAgent_PATH)
#===========================================================

#===========================SIPAG===========================
#SIPAG_PATH= $(TOP_DIR)/sipag
#SIPAGCSRCS= acl.c \
			frame.c \
			loader.c \
			translate.c \
			ag_sip.c \
			rtp.c

#SIPAG_COBJS := $(SIPAGCSRCS:%.c=%.o)

#SIPAG_PATHCSRCS :=$(SIPAG_CSRCS:$(SIPAG_PATH)/%)
#SIPAG_PATHCOBJS :=$(SIPAG_COBJS:%=$(TARGET_TEMPDIR)/%)
#===========================================================


$(TARGET): $(MAIN_PATHCOBJS) $(AC491_PATHCOBJS) $(paraInjectAgent_PATHCOBJS) $(EXTERN_OBJ) $(BASIC_PATHCOBJS)
	$(CC) $(MAIN_PATHCOBJS) $(AC491_PATHCOBJS) $(paraInjectAgent_PATHCOBJS) $(EXTERN_OBJ) $(BASIC_PATHCOBJS)  -o $(TARGET_PROG) $(LNKFLAG)
	
$(MAIN_PATHCOBJS): $(MAIN_PATHCSRCS)
	$(PRE_CC) $(CC) $(CFLAG) -c -o $@ $(MAIN_PATH)/$(*F).c

$(AC491_PATHCOBJS): $(AC491_PATHCSRCS)
	$(PRE_CC) $(CC) $(CFLAG) -c -o $@ $(AC491_PATH)/$(*F).c

$(paraInjectAgent_PATHCOBJS): $(paraInjectAgent_PATHCSRCS)
	$(PRE_CC) $(CC) $(CFLAG) -c -o $@ $(paraInjectAgent_PATH)/$(*F).c

$(BASIC_PATHCOBJS): $(BASIC_PATHCSRCS)
	$(PRE_CC) $(CC) $(CFLAG) -c -o $@ $(SRC_PATH)/$(*F).c

#$(SIPAG_PATHCOBJS): $(SIPAG_PATHCSRCS)
#	$(PRE_CC) $(CC) $(CFLAG) -c -o $@ $(SIPAG_PATH)/$(*F).c

install:
	mkdir -p $(TARGET_TEMPDIR)

#SIPAG_LIB := $(TARGET_TEMPDIR)/sipag.a

#sipaglib:$(BASIC_PATHCOBJS) $(SIPAG_PATHCOBJS)
#	$(AR) $(SIPAG_LIB) $(BASIC_PATHCOBJS) $(SIPAG_PATHCOBJS)

clean:
	rm -f $(TARGET_TEMPDIR)/*.o $(SIPAG_LIB)
	rm -f $(TARGET_PROG)
