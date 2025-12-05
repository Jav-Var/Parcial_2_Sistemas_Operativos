# Makefile

CC ?= gcc
OBJDIR := build/obj
BINDIR := build

CPPFLAGS ?= -I./src               
CFLAGS   ?= -std=c11 -O2 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -MMD -MP
LDFLAGS  ?=
LDLIBS   ?= -lm -lpthread

AGENT_SRCS     := src/agent/main.c
COLLECTOR_SRCS := src/collector/main.c src/collector/parser.c

AGENT_OBJS     := $(patsubst src/%.c,$(OBJDIR)/%.o,$(AGENT_SRCS))
COLLECTOR_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(COLLECTOR_SRCS))

.PHONY: all clean distclean agent collector

all: agent collector

agent: $(BINDIR)/agent
collector: $(BINDIR)/collector

$(BINDIR)/agent: $(AGENT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/collector: $(COLLECTOR_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Compile rule: src/path/file.c -> build/obj/path/file.o
$(OBJDIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Include dependency files if they exist
-include $(AGENT_OBJS:.o=.d)
-include $(COLLECTOR_OBJS:.o=.d)

clean:
	$(RM) $(AGENT_OBJS) $(COLLECTOR_OBJS) \
	        $(AGENT_OBJS:.o=.d) $(COLLECTOR_OBJS:.o=.d)

distclean: clean
	$(RM) -r $(BINDIR)
