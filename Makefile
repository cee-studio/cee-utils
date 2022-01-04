CC ?= gcc

OBJDIR     := obj
TEST_DIR   := test

SRC_C89 := ntl.c \
           json-string.c \
           json-parser.c 

SRC_C99 := cee-utils.c \
           json-actor-boxed.c \
           json-actor.c \
           log.c \
           logconf.c

SRC_NOSTD := json-struct.c

OBJS_C89   := $(SRC_C89:%.c=$(OBJDIR)/%.o)
OBJS_C99   := $(SRC_C99:%.c=$(OBJDIR)/%.o)
OBJS_NOSTD := $(SRC_NOSTD:%.c=$(OBJDIR)/%.o)

CFLAGS += -O0 -g \
          -I. -DLOG_USE_COLOR

ifneq ($(release),1)
	CFLAGS += -D_STATIC_DEBUG
endif

ifeq ($(DEBUG_JSON),1)
	CFLAGS += -D_STRICT_STATIC_DEBUG
endif

WFLAGS += -Wall -Wextra -pedantic

all: $(OBJS_C89) $(OBJS_C99)

test: all
	$(MAKE) -C $(TEST_DIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)

$(OBJS_C89): $(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -std=c89 $(CFLAGS) $(WFLAGS) -c -o $@ $<
$(OBJS_C99): $(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) -std=c99 $(CFLAGS) $(WFLAGS) -c -o $@ $<
$(OBJS_NOSTD): $(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(CFLAGS) $(WFLAGS) -c -o $@ $<

echo:
	@ echo -e 'SRC: $(SRC)'
	@ echo -e 'OBJS_C89: $(OBJS_C89)'
	@ echo -e 'OBJS_C99: $(OBJS_C99)'
	@ echo -e 'OBJS_NOSTD: $(OBJS_NOSTD)'

clean: 
	rm -rf $(OBJDIR)
	$(MAKE) -C $(TEST_DIR) clean

.PHONY : all test echo clean
