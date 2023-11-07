# Toolchain settings
TARGET  := 
CC      := $(TARGET)gcc
CXX     := $(TARGET)g++
AS      := $(TARGET)as
LD      := $(TARGET)gcc
GDB     := $(TARGET)gdb

BUILD   ?= release

# User Settings
BINNAME     = pi.out
PKG_CONFIG	=
AINCDIR     = 
INCDIR      = 
LIBS        = m
FLAGS       = -march=x86-64-v3
CFLAGS      = -std=c11
CXXFLAGS    = -std=c++17
LDFLAGS     = -Wl,--gc-sections -pthread
DEFFLAGS    = 
DBGFLAGS    = -pg -g -DDEBUG
LDSCRIPT    = 

# Compile Flags
__FLAGS      = $(if $(PKG_CONFIG), $(shell pkg-config --cflags $(PKG_CONFIG)))
__ASFLAGS    = $(foreach d, $(AINCDIR), -I$d)
__CFLAGS     = $(OPTIFLAGS) $(DEFFLAGS) $(foreach d, $(INCDIR), -I$d) $(CFLAGS)
__CXXFLAGS   = $(OPTIFLAGS) $(DEFFLAGS) $(foreach d, $(INCDIR), -I$d) $(CXXFLAGS)
__LDFLAGS    = $(if $(PKG_CONFIG), $(shell pkg-config --libs $(PKG_CONFIG))) $(foreach l, $(LIBS), -l$l) $(LDFLAGS)

OPTIFLAGS   = 
ifeq ($(BUILD),debug)
	OBJDIR = obj/Debug
	DEPDIR = dep/Debug
	__FLAGS += $(DBGFLAGS)
	OPTIFLAGS = -O0
else ifeq ($(BUILD),release)
	OBJDIR = obj/Release
	DEPDIR = dep/Release
	OPTIFLAGS = -O2
	__FLAGS += -DNDEBUG
else
	OBJDIR = $(error "Unknown build type")
endif

ifneq ($(LDSCRIPT),)
	__LDFLAGS += -T $(LDSCRIPT)
endif

SRCS.s      = $(wildcard *.s)
SRCS.c      = $(wildcard *.c)
SRCS.cpp    = $(wildcard *.cpp)
OBJFILES    = $(SRCS.c:.c=.o) $(SRCS.cpp:.cpp=.o) $(SRCS.s:.s=.o)
OBJS        = $(addprefix $(OBJDIR)/,$(OBJFILES))
DEPFILES    = $(SRCS.c:.c=.d) $(SRCS.cpp:.cpp=.d)
DEPS        = $(addprefix $(DEPDIR)/,$(DEPFILES))

.PHONY: clean distclean gdb

all: $(BINNAME)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(DEPDIR):
	mkdir -p $(DEPDIR)

$(OBJDIR)/%.o: %.s | $(OBJDIR)
	$(AS) $(FLAGS) $(__ASFLAGS) -o $@ $<

$(OBJDIR)/%.o: %.c | $(OBJDIR)
	$(CC) $(FLAGS) $(__FLAGS) $(__CFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(FLAGS) $(__FLAGS) $(__CXXFLAGS) -c -o $@ $<

$(DEPDIR)/%.d: %.c | $(DEPDIR)
	@echo -n "$(OBJDIR)/" > $@
	@$(CC) -MM -MP $(FLAGS) $(__CFLAGS) $< >> $@

$(DEPDIR)/%.d: %.cpp | $(DEPDIR)
	@echo -n "$(OBJDIR)/" > $@
	@$(CXX) -MM -MP $(FLAGS) $(__CXXFLAGS) $< >> $@

$(BINNAME): $(OBJS)
	$(LD) $(FLAGS) $(__LDFLAGS) -o $@ $(OBJS)

ifneq ($(DEPS),)
  sinclude $(DEPS)
endif

gdb: $(BINNAME)
	$(GDB) -se $(BINNAME)

clean:
	rm -rf obj
	rm -f *.o $(BINNAME)

distclean: clean
	rm -rf dep
	rm -f *.d
