# Project Name
TARGET = patch_init_chuck

# ChucK core is ~1.2 MB, too large for internal flash; run from QSPI via the Daisy bootloader
APP_TYPE = BOOT_QSPI

# Library Locations
LIBDAISY_DIR ?= ../libDaisy
CHUCK_DIR ?= chuck

# Sources
CHUCK_CPP_SOURCES = $(wildcard $(CHUCK_DIR)/*.cpp)
CHUCK_C_SOURCES = $(wildcard $(CHUCK_DIR)/*.c)

CPP_SOURCES = patch_init_chuck.cpp sdram_heap.cpp $(CHUCK_CPP_SOURCES)
C_SOURCES = $(CHUCK_C_SOURCES)

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
