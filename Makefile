CC     ?= gcc
OBJDIR := obj
SHELL  := /bin/bash
ACC    ?= gcc

SRC := $(wildcard *.c)
OBJS := $(SRC:%.c=%.o)

TEST_SRC  := $(wildcard test/test-*.c)
TEST_EXES := $(TEST_SRC:%.c=%.exe)

CFLAGS += -Wall -std=c11 -O0 -g \
	-Wno-unused-function -Wno-unused-but-set-variable \
	-I. -DLOG_USE_COLOR
ifneq ($(release),1)
	CFLAGS += -D_STATIC_DEBUG
endif
ifeq ($(DEBUG_JSON),1)
	CFLAGS += -D_STRICT_STATIC_DEBUG
endif
ifeq ($(CC),stensal-c)
	CFLAGS += -D_DEFAULT_SOURCE
else
	CFLAGS += -fPIC -D_XOPEN_SOURCE=700
endif


PREFIX ?= /usr/local

.PHONY : all clean
.ONESHELL :


all : mkdir $(OBJS) $(TEST_EXES)

echo :
	@echo SRC $(SRC)
	@echo OBJS $(OBJS)
	@echo TEST_SRC $(TEST_SRC)
	@echo TEST_EXES $(TEST_EXES)

mkdir :
	mkdir -p $(OBJDIR)
	mkdir -p $(OBJDIR)/test

#generic compilation
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) $(LIBS_CFLAGS) -c -o $@ $< $(LIBS_LDFLAGS)

%.exe : %.c
	$(CC) $(CFLAGS) $(LIBS_CFLAGS) -o $@ $< $(LIBS_LDFLAGS)

clean : 
	rm -rf $(OBJDIR) test/*.exe
