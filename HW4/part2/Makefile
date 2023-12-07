# Toolchain settings
TARGET  := 
CC      := $(TARGET)gcc
CXX     := $(TARGET)g++
AS      := $(TARGET)as
LD      := $(TARGET)g++
GDB     := $(TARGET)gdb

BUILD   ?= release

# User Settings
BINNAME     = matmul
PKG_CONFIG	=
AINCDIR     = 
INCDIR      = /usr/lib/x86_64-linux-gnu/openmpi/include/openmpi /usr/lib/x86_64-linux-gnu/openmpi/include
LIBS        = m mpi mpi_cxx
FLAGS       = -march=native -pthread
CFLAGS      = -std=c11
CXXFLAGS    = -std=c++17
LDFLAGS     = -Wl,--gc-sections -L/usr/lib/x86_64-linux-gnu/openmpi/lib
DEFFLAGS    = 
DBGFLAGS    = -g -DDEBUG
LDSCRIPT    = 

# Compile Flags
__FLAGS      = $(shell pkg-config --cflags $(PKG_CONFIG))
__ASFLAGS    = $(foreach d, $(AINCDIR), -I$d)
__CFLAGS     = $(OPTIFLAGS) $(DEFFLAGS) $(foreach d, $(INCDIR), -I$d) $(CFLAGS)
__CXXFLAGS   = $(OPTIFLAGS) $(DEFFLAGS) $(foreach d, $(INCDIR), -I$d) $(CXXFLAGS)
__LDFLAGS    = $(shell pkg-config --libs $(PKG_CONFIG)) $(LDFLAGS) $(foreach l, $(LIBS), -l$l)

OPTIFLAGS   = 
ifeq ($(BUILD),debug)
	OBJDIR = obj/Debug
	DEPDIR = dep/Debug
	__FLAGS += $(DBGFLAGS)
	OPTIFLAGS = -O0
else ifeq ($(BUILD),release)
	OBJDIR = obj/Release
	DEPDIR = dep/Release
	OPTIFLAGS = -O3
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
SRCS.cc     = $(wildcard *.cc)
OBJFILES    = $(SRCS.c:.c=.o) $(SRCS.cpp:.cpp=.o) $(SRCS.cc:.cc=.o) $(SRCS.s:.s=.o)
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

$(OBJDIR)/%.o: %.cc | $(OBJDIR)
	$(CXX) $(FLAGS) $(__FLAGS) $(__CXXFLAGS) -c -o $@ $<

$(OBJDIR)/%.o: %.cpp | $(OBJDIR)
	$(CXX) $(FLAGS) $(__FLAGS) $(__CXXFLAGS) -c -o $@ $<

$(DEPDIR)/%.d: %.c | $(DEPDIR)
	@echo -n "$(OBJDIR)/" > $@
	@$(CC) -MM -MP $(FLAGS) $(__CFLAGS) $< >> $@

$(DEPDIR)/%.d: %.cc | $(DEPDIR)
	@echo -n "$(OBJDIR)/" > $@
	@$(CXX) -MM -MP $(FLAGS) $(__CXXFLAGS) $< >> $@

$(DEPDIR)/%.d: %.cpp | $(DEPDIR)
	@echo -n "$(OBJDIR)/" > $@
	@$(CXX) -MM -MP $(FLAGS) $(__CXXFLAGS) $< >> $@

$(BINNAME): $(OBJS)
	$(LD) $(FLAGS) -o $@ $(OBJS) $(__LDFLAGS)

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
