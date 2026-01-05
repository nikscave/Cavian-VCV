RACK_DIR ?= $(RACK_SDK_DIR)
FLAGS +=
CFLAGS +=
CXXFLAGS +=
LDFLAGS += -lws2_32
SOURCES += $(wildcard src/*.cpp)
DISTRIBUTABLES += res
include $(RACK_DIR)/plugin.mk
