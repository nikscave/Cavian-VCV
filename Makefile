RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

include $(RACK_DIR)/plugin.mk

# Add libraries AFTER plugin.mk (it might clear LDFLAGS before)
ifdef ARCH_WIN
	LDFLAGS += -lws2_32
endif

ifdef ARCH_LIN
	LDFLAGS += -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -lpthread
endif
