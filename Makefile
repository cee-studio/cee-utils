CC     ?= gcc
OBJDIR := obj

SRC := $(wildcard *.c)
OBJS := $(SRC:%.c=$(OBJDIR)/%.o)

TEST_SRC  := $(wildcard test/test-*.c)
TEST_EXES := $(TEST_SRC:%.c=%.exe)

CFLAGS += -Wall -std=c11 -O0 -g \
	-Wno-unused-function \
	-I./ -DLOG_USE_COLOR
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

LDFLAGS += -lm

.PHONY : all test clean

# generic compilation
$(OBJDIR)/%.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $<
	
test/test-json-actor.exe : test/test-json-actor.c
	$(CC) $< $(filter-out $(OBJDIR)/json-actor.o, $(OBJS)) \
		$(CFLAGS) -o $@ $(LDFLAGS)
# generic compilation
test/test-%.exe : test/test-%.c
	$(CC) $< $(OBJS) $(CFLAGS) -o $@ $(LDFLAGS)

all : test

echo :
	@echo SRC $(SRC)
	@echo OBJS $(OBJS)
	@echo TEST_SRC $(TEST_SRC)
	@echo TEST_EXES $(TEST_EXES)

test : $(TEST_EXES)

$(TEST_EXES) : $(TEST_EXES) | $(OBJS)
$(OBJS): | $(OBJDIR)

$(OBJDIR) :
	mkdir -p $(OBJDIR)/test

clean : 
	rm -rf $(OBJDIR) test/*.exe
