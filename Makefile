# Project Name
TARGET = BassPedal

# Sources
CPP_SOURCES = BassPedal.cpp

# Library Locations
DAISYSP_DIR ?= ../../DaisyExamples/DaisySP
LIBDAISY_DIR ?= ../../DaisyExamples/libDaisy

# Core location, and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

