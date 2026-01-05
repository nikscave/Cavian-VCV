RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Only link ws2_32 on Windows
ifeq ($(ARCH_OS), win)
	LDFLAGS += -lws2_32
endif

SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res
include $(RACK_DIR)/plugin.mk
