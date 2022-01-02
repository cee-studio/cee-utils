CC ?= gcc

OBJDIR     := obj
TEST_DIR   := test

SRC := $(wildcard *.c)

OBJS := $(SRC:%.c=$(OBJDIR)/%.o)

CFLAGS += -O0 -g              \
          -I. -DLOG_USE_COLOR

ifneq ($(release),1)
	CFLAGS += -D_STATIC_DEBUG
endif

ifeq ($(DEBUG_JSON),1)
	CFLAGS += -D_STRICT_STATIC_DEBUG
endif

WFLAGS += -Wall -Wextra -pedantic

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(WFLAGS) -c -o $@ $<

all: $(OBJS)

test: all
	$(MAKE) -C $(TEST_DIR)

$(OBJS): | $(OBJDIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

echo:
	@ echo -e 'SRC: $(SRC)'
	@ echo -e 'OBJS: $(OBJS)'

clean: 
	rm -rf $(OBJDIR)
	$(MAKE) -C $(TEST_DIR) clean

.PHONY : all test echo clean
