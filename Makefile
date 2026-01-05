RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=

SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

# Add Windows socket library after plugin.mk sets ARCH_WIN
ifdef ARCH_WIN
	LDFLAGS += -lws2_32
endif
