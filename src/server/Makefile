# src/server/Makefile

CXX := g++

CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -fsanitize=address
CXX_DEBUG_FLAGS   := -g3 -ggdb3
CXX_RELEASE_FLAGS := -O3
CXXFLAGS += $(CXX_DEBUG_FLAGS)

LDFLAGS := -fsanitize=address
LIBS := fmt spdlog
LIB_FLAGS := $(addprefix -l,$(LIBS))
LDFLAGS += $(LIB_FLAGS)

BUILD_DIR := ../../build
SRC_DIR   := ../../src

# find all server-related .cc under src, excluding client code
SERVER_SRCS := $(shell find $(SRC_DIR) -type f -name '*.cc' -not -path '*/client/*' -not -name 'client_main.cc')
SERVER_OBJS := $(patsubst $(SRC_DIR)/%.cc,$(BUILD_DIR)/%.o,$(SERVER_SRCS))


ALL_OBJS := $(sort $(SERVER_OBJS))
DEPS := $(ALL_OBJS:.o=.d)

INC_DIRS  := $(shell find $(SRC_DIR) -type d) \
			 server \
			 ../utils
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP

.PHONY: all
all: $(BUILD_DIR)/server-bin

$(BUILD_DIR)/server-bin: $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# compile any .cc into build/%.o (preserving directory structure)
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cc
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: print-vars
print-vars:
	@echo "SERVER_SRCS   = $(SERVER_SRCS)"
	@echo "SERVER_OBJS   = $(SERVER_OBJS)"
	@echo "ALL_OBJS      = $(ALL_OBJS)"
	@echo "DEPS          = $(DEPS)"
	@echo "INC_FLAGS     = $(INC_FLAGS)"

.PHONY: clean
clean:
	rm -rf build

-include $(DEPS)


