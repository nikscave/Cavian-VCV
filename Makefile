RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res

# Add library flags BEFORE including plugin.mk
ifdef ARCH_LIN
	# Option 1: If OpenSSL is in the toolchain, link statically
	LDFLAGS += -Wl,-Bstatic -lssl -lcrypto -Wl,-Bdynamic -lpthread
	
	# Option 2: If you need dynamic linking (uncomment instead of Option 1)
	# LDFLAGS += -lpthread -lssl -lcrypto
	# CXXFLAGS += -I/usr/include/openssl
	# LDFLAGS += -L/usr/lib/x86_64-linux-gnu
endif

ifdef ARCH_WIN
	LDFLAGS += -Wl,-Bstatic -l:libpthread.a -Wl,-Bdynamic -lws2_32
endif

include $(RACK_DIR)/plugin.mk
