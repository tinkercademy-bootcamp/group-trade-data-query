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

BUILD_DIR := build
SRC_DIR   := ../../src

# find all server-related .cc under src, excluding client code
SRCS := $(shell find $(SRC_DIR) -type f -name '*.cc' -not -path '*/client/*' -not -name 'client_main.cc')

OBJS          := $(SRCS:%=$(BUILD_DIR)/%.o)
NON_MAIN_OBJS := $(filter-out %server-main.cc.o,$(OBJS))
DEPS          := $(OBJS:.o=.d)

INC_DIRS  := $(shell find $(SRC_DIR) -type d) \
             server \
             ../utils
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS := $(INC_FLAGS) -MMD -MP

.PHONY: all
all: $(BUILD_DIR)/server

$(BUILD_DIR)/server: $(BUILD_DIR)/../server-main.cc.o $(NON_MAIN_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# compile any .cc into build/%.cc.o (preserving directory structure)
$(BUILD_DIR)/%.cc.o: %.cc
	mkdir -p $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY: print-vars
print-vars:
	@echo "SRCS          = $(SRCS)"
	@echo "OBJS          = $(OBJS)"
	@echo "NON_MAIN_OBJS = $(NON_MAIN_OBJS)"
	@echo "DEPS          = $(DEPS)"
	@echo "INC_FLAGS     = $(INC_FLAGS)"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
