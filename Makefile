CC ?= gcc
OBJDIR := build/obj
BINDIR := build

CPPFLAGS ?= -I./src -I./src/collector -I./src/common
CFLAGS   ?= -std=c11 -O2 -D_POSIX_C_SOURCE=200112L -g -Wall -Wextra -MMD -MP
LDFLAGS  ?=
LDLIBS   ?= -lm -lpthread

AGENT_SRCS 	:= src/agent/agent.c
COLLECTOR_SRCS 	:= src/collector/collector.c src/collector/handle_host.c src/common/common.c  
VISUALIZER_SRCS := src/visualizer/visualizer.c src/visualizer/table_display.c src/common/common.c

AGENT_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(AGENT_SRCS))
COLLECTOR_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(COLLECTOR_SRCS))
VISUALIZER_OBJS := $(patsubst src/%.c,$(OBJDIR)/%.o,$(VISUALIZER_SRCS))

.PHONY: all clean distclean agent_mem agent_cpu collector

all: agent collector visualizer

#agent_cpu: $(BINDIR)/agent_cpu
#agent_mem: $(BINDIR)/agent_mem
agent: $(BINDIR)/agent
collector: $(BINDIR)/collector
visualizer: $(BINDIR)/visualizer

#$(BINDIR)/agent_mem: $(AGENT_MEM_OBJS)
#	@mkdir -p $(dir $@)
#	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

#$(BINDIR)/agent_cpu: $(AGENT_CPU_OBJS)
#	@mkdir -p $(dir $@)
#	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/agent: $(AGENT_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/collector: $(COLLECTOR_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(BINDIR)/visualizer: $(VISUALIZER_OBJS)
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
	$(RM) $(AGENT_OBJS) $(COLLECTOR_OBJS) $(VISUALIZER_OBJS)\
	        $(AGENT_OBJS:.o=.d) $(COLLECTOR_OBJS:.o=.d) $(VISUALIZER_OBJS:.o=.d)

distclean: clean
	$(RM) -r $(BINDIR)
