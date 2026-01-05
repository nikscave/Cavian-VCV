RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

# Force static linking and add Windows socket library
ifdef ARCH_WIN
	LDFLAGS += -static-libgcc -static-libstdc++ -static -lws2_32
endif
