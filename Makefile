# Makefile for Amiga Sampler
OS := $(shell uname)
CPU_FLAGS = -m68000 -mnobitfield -fno-peephole2

# Directories
LIBDIR = /opt/amiga/m68k-amigaos/lib
SDKDIR = /opt/amiga/m68k-amigaos/sys-include
NDKDIR = /opt/amiga/m68k-amigaos/ndk-include
INCDIR = /opt/amiga/m68k-amigaos/include
SRCDIR = src
BUILDDIR = build/os3/obj
OUTDIR = out

# Program settings
CC = m68k-amigaos-gcc
PROGRAM_NAME = AmigaSampler

# Source files
SOURCES = $(SRCDIR)/main.c \
          $(SRCDIR)/gui/gui.c 

# Object files
OBJECTS = $(BUILDDIR)/main.o \
          $(BUILDDIR)/gui/gui.o 

# Compiler flags
BASE_CCFLAGS = -MP -MMD -Wextra -Wno-unused-function \
 -Wno-discarded-qualifiers -Wno-int-conversion \
 -Wno-volatile-register-var -fno-lto -noixemul \
 -fbaserel -lamiga -lmui -D__AMIGAOS3__ $(CPU_FLAGS) \
 -I$(INCDIR) -I$(SDKDIR) -I$(NDKDIR) -I$(SRCDIR)/include

ifdef DEBUG
CCFLAGS = $(BASE_CCFLAGS) -DDEBUG_BUILD -O0 -g
BUILD_TYPE = debug
else
CCFLAGS = $(BASE_CCFLAGS) -O2 -fomit-frame-pointer
BUILD_TYPE = release
endif

LDFLAGS = -Wl,-Map=$(OUTDIR)/$(PROGRAM_NAME).map,-L$(LIBDIR),-lamiga,-lmui

# Targets
.PHONY: all clean debug release dirs

all: release

debug:
	@echo "Building debug version..."
	$(MAKE) dirs
	$(MAKE) $(OUTDIR)/$(PROGRAM_NAME) DEBUG=1

release:
	@echo "Building release version..."
	$(MAKE) dirs
	$(MAKE) $(OUTDIR)/$(PROGRAM_NAME)

dirs:
	@mkdir -p $(BUILDDIR)/gui
	@mkdir -p $(OUTDIR)

$(OUTDIR)/$(PROGRAM_NAME): $(OBJECTS)
	$(info Linking $(PROGRAM_NAME) ($(BUILD_TYPE) build))
	$(CC) $(CCFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(info Compiling $< ($(BUILD_TYPE) build))
	$(CC) $(CCFLAGS) -c -o $@ $<

$(BUILDDIR)/gui/%.o: $(SRCDIR)/gui/%.c
	@mkdir -p $(dir $@)
	$(info Compiling $< ($(BUILD_TYPE) build))
	$(CC) $(CCFLAGS) -c -o $@ $<

clean:
	$(info Cleaning...)
	@$(RM) -rf $(BUILDDIR)
	@$(RM) -f $(OUTDIR)/$(PROGRAM_NAME)
	@$(RM) -f $(OUTDIR)/$(PROGRAM_NAME).map

debug-print:
	@echo "Source Directory: $(SRCDIR)"
	@echo "Build Directory: $(BUILDDIR)"
	@echo "Object Files: $(OBJECTS)"
	@echo "CCFLAGS: $(CCFLAGS)"
	@echo "LDFLAGS: $(LDFLAGS)"

-include $(OBJECTS:.o=.d)