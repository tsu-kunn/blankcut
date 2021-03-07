SHELL       = /bin/bash

TOPDIR      = .
INCDIR      = -I$(TOPDIR)/include
STUBDIR     = 

TARGET      = blankcut
OBJS        = blankcut.o ¥
              blank.o ¥
              mto_memctrl.o
LIB_OBJS    = $(OBJS)
PRX_OBJS    = $(OBJS)

AS          = gcc
CC          = gcc
LD          = gcc
AR          = ar
RANLIB      = ranlib
OBJDUMP     = objdump

ifndef NDEBUG
CFLAGS      = -std=c11 -O0 -fexceptions -Wuninitialized -Wold-style-cast
else
CFLAGS      = -std=c11 -O3 -fexceptions -Wuninitialized -DNDEBUG
endif

ASFLAGS     = -c -xassembler-with-cpp
LDFLAGS     = -Wl,--warn-common,--warn-constructors,--warn-multiple-gp


all: $(TARGET)

$(TARGET): $(OBJS)

.c.o:
    $(CC) $(CFLAGS) $(TMPFLAGS) $(INCDIR) -Wa,-al=$*.lst -c $< -o $*.o


clean:
    @$(RM) *.o *.map *.lst
    @$(RM) $(TARGET)
    @$(RM) *.bak


install: all


