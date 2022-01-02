CC ?= gcc

OBJDIR     := obj
TEST_DIR   := test
THIRDP_DIR := third-party

THIRDP_SRC := $(wildcard $(THIRDP_DIR)/*.c)
SRC        := $(wildcard *.c)

export OBJS := $(SRC:%.c=$(OBJDIR)/%.o) $(THIRDP_SRC:$(THIRDP_DIR)/%.c=$(OBJDIR)/%.o)

CFLAGS += -O0 -g                              \
          -I. -I$(THIRDP_DIR) -DLOG_USE_COLOR

ifneq ($(release),1)
	CFLAGS += -D_STATIC_DEBUG
endif

ifeq ($(DEBUG_JSON),1)
	CFLAGS += -D_STRICT_STATIC_DEBUG
endif

WFLAGS += -Wall -Wextra -pedantic

$(OBJDIR)/%.o: $(THIRDP_DIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
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
