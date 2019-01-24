# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..
SLUG = mdh-modules
VERSION = 1.0.0

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=-Idep/include
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += $(wildcard LICENSE*) res

$(shell mkdir -p dep)
DEP_LOCAL := dep

# Include the VCV Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk
