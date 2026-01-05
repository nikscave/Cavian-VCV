RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

# Force static linking of GCC/pthread runtime libraries and add Windows socket library
# Use += to append after plugin.mk's flags
ifdef ARCH_WIN
	LDFLAGS := $(LDFLAGS) -Wl,-Bstatic -lpthread -Wl,-Bdynamic -lws2_32
endif
