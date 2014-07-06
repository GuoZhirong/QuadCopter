
CROSS_COMPILE     ?= arm-none-eabi-
DEBUG             ?= 0

REV               ?= E

#OpenOCD conf
RTOS_DEBUG        ?= 0

############### Location configuration ################
FREERTOS = lib/FreeRTOS
PORT = $(FREERTOS)/portable/GCC/ARM_CM3
STLIB = lib/

################ Build configuration ##################
# St Lib
VPATH += $(STLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/startup/gcc_ride7/
CRT0=startup_stm32f10x_md.o
VPATH += $(STLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x/
CRT0+= system_stm32f10x.o

include scripts/st_obj.mk

# FreeRTOS
VPATH += $(PORT)
PORT_OBJ=port.o
VPATH +=  $(FREERTOS)/portable/MemMang
MEMMANG_OBJ = heap_4.o

VPATH += $(FREERTOS)
FREERTOS_OBJ = list.o tasks.o queue.o timers.o $(MEMMANG_OBJ)
FREERTOS_OBJ += freeRTOSdebug.o
VPATH += driver/src main

PROJ_OBJ = main.o

OBJ = $(CRT0) $(ST_OBJ) $(PROJ_OBJ) $(FREERTOS_OBJ) $(PORT_OBJ)

ifdef P
  C_PROFILE = -D P_$(P)
endif

############### Compilation configuration ################
AS = $(CROSS_COMPILE)as
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)gcc
SIZE = $(CROSS_COMPILE)size
OBJCOPY = $(CROSS_COMPILE)objcopy

INCLUDES = -I$(FREERTOS)/include -I$(PORT) -I.
INCLUDES+= -I$(STLIB)/STM32F10x_StdPeriph_Driver/inc
INCLUDES+= -I$(STLIB)/CMSIS/CM3/CoreSupport
INCLUDES+= -I$(STLIB)/CMSIS/CM3/DeviceSupport/ST/STM32F10x
INCLUDES+= -Iconfig -Imain/inc -Idrivers/inc

PROCESSOR = -mcpu=cortex-m3 -mthumb

#Flags required by the ST library
STFLAGS = -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=12000000 -include stm32f10x_conf.h

ifeq ($(DEBUG), 1)
  CFLAGS += -O0 -g3
else
  CFLAGS += -Os -g3
endif


CFLAGS += -DBOARD_REV_$(REV)

CFLAGS += $(PROCESSOR) $(INCLUDES) $(STFLAGS) -Wall -fno-strict-aliasing $(C_PROFILE)


#Permits to remove un-used functions and global variables from output file
CFLAGS += -ffunction-sections -fdata-sections

ASFLAGS = $(PROCESSOR) $(INCLUDES)
LDFLAGS = $(CFLAGS) -Wl,-Map=$(PROG).map,--cref,--gc-sections -nostdlib

LDFLAGS += -T scripts/STM32_Ram20k_Flash32k.ld

#Program name
PROG = bin/controller
#Where to compile the .o
BIN = bin/obj
VPATH += $(BIN)

#Dependency files to include
DEPS := $(foreach o,$(OBJ),$(BIN)/dep/$(o).d)

##################### Misc. ################################
ifeq ($(SHELL),/bin/sh)
  COL_RED=/033[1;31m
  COL_GREEN=/033[1;32m
  COL_RESET=/033[m
endif

#################### Targets ###############################


all: build
build: compile size
compile: $(PROG).hex $(PROG).bin


size: compile
	@$(SIZE) -B $(PROG).elf

#Print preprocessor #defines
prep:
	@$(CC) -dD

include scripts/targets.mk

#include dependencies
-include $(DEPS)
