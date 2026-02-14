RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

# Add libraries AFTER plugin.mk
ifdef ARCH_WIN
	LDFLAGS += -lws2_32
endif

ifdef ARCH_LIN
	LDFLAGS += -lpthread
endif
