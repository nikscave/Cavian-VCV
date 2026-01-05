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

# Force universal binary on macOS
ifdef ARCH_MAC
	FLAGS += -arch x86_64 -arch arm64
endif
