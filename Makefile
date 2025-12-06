CC ?= gcc
OBJDIR := build/obj
BINDIR := build

CPPFLAGS ?= -I./src               
CFLAGS   ?= -std=c11 -O2 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -MMD -MP
LDFLAGS  ?=
LDLIBS   ?= -lm -lpthread

AGENT_MEM_SRCS :=  src/agents/agent_mem.c
AGENT_CPU_SRCS :=  src/agents/agent_cpu.c
COLLECTOR_SRCS := src/collector/main.c src/collector/parser.c


AGENT_MEM_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(AGENT_MEM_SRCS))
AGENT_CPU_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(AGENT_CPU_SRCS))
COLLECTOR_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(COLLECTOR_SRCS))

.PHONY: all clean distclean agent_mem agent_cpu collector

all: agent_mem agent_cpu collector

agent_cpu: $(BINDIR)/agent_cpu
agent_mem: $(BINDIR)/agent_mem
collector: $(BINDIR)/collector

$(BINDIR)/agent_mem: $(AGENT_MEM_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/agent_cpu: $(AGENT_CPU_OBJS)
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
