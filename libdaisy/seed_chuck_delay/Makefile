# Project Name
TARGET = seed_chuck_delay

# ChucK core is ~1.2 MB, too large for internal flash; run from QSPI via the Daisy bootloader
APP_TYPE = BOOT_QSPI

# Library Locations
LIBDAISY_DIR ?= ../libDaisy
CHUCK_DIR ?= ../chuck
MPLSARTINDUSTRY_DIR ?= ../mplsartindustry

# Sources
CHUCK_CPP_SOURCES = $(filter-out $(CHUCK_DIR)/sdram_heap.cpp,$(wildcard $(CHUCK_DIR)/*.cpp))
CHUCK_C_SOURCES = $(wildcard $(CHUCK_DIR)/*.c)

# Note: add sketch sources here
CPP_SOURCES = seed_chuck_delay.cpp $(MPLSARTINDUSTRY_DIR)/mplsartindustry_seed.cpp $(CHUCK_DIR)/sdram_heap.cpp $(CHUCK_CPP_SOURCES)
C_SOURCES = $(CHUCK_C_SOURCES)

# MA&I pcb board
C_INCLUDES += -I$(MPLSARTINDUSTRY_DIR)

# Note: add ChucK source here
# ChucK program, wrapped in a C++ raw string literal at build time
CHUCK_PROGRAM = seed_chuck_delay.ck
C_INCLUDES += -I$(BUILD_DIR)

# ChucK core, embedded configuration
C_INCLUDES += -I$(CHUCK_DIR)
C_DEFS += -D__CHUCK_EMBEDDED__
CPP_STANDARD = -std=gnu++17

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

# silence warnings from upstream ChucK sources
CHUCK_OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(CHUCK_CPP_SOURCES:.cpp=.o) $(CHUCK_C_SOURCES:.c=.o)))
$(CHUCK_OBJECTS): CFLAGS += -w

# generate $(BUILD_DIR)/seed_chuck_delay_ck.h from the ChucK program
$(BUILD_DIR)/seed_chuck_delay_ck.h: $(CHUCK_PROGRAM) Makefile | $(BUILD_DIR)
	{ printf 'R"CHUCK('; cat $<; printf ')CHUCK"\n'; } > $@

$(BUILD_DIR)/seed_chuck_delay.o: $(BUILD_DIR)/seed_chuck_delay_ck.h
