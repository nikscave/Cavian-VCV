RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

# Windows-specific linking for sockets
ifdef ARCH_WIN
	LDFLAGS += -lws2_32
endif

# Linux-specific linking for HTTP/TLS support
ifdef ARCH_LIN
	LDFLAGS += -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -lpthread
endif

include $(RACK_DIR)/plugin.mk
