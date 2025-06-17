CXX := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -pedantic -fsanitize=address
CXX_DEBUG_FLAGS := -g3 -ggdb3
CXX_RELEASE_FLAGS := -O3
TEST_FLAGS := -DTESTMODE

LDFLAGS := -lspdlog -lfmt

CSVS := $(wildcard data/raw/*.csv)

SRC_DIR := src
SRC_CLIENT := $(SRC_DIR)/client
SRC_SERVER := $(SRC_DIR)/server
SRC_QUERY_ENGINE := $(SRC_DIR)/query_engine

CLIENT_SRC := $(shell find src/client -type f -name '*.cc')
CLIENT_OBJS := $(patsubst src/client/%.cc,build/client/%.o,$(CLIENT_SRC))
CLIENT_TEST_OBJS := $(patsubst src/client/%.cc,build/client-test/%.o,$(CLIENT_SRC))
CLIENT_HEADERS := $(shell find src/client -type f -name '*.h') $(shell find src/utils -type f -name '*.h')

SERVER_SRC := $(shell find src/server -type f -name '*.cc') $(shell find src/query_engine -type f -name '*.cc')
SERVER_OBJS := $(patsubst src/server/%.cc,build/server/%.o,$(patsubst src/query_engine/%.cc,build/query_engine/%.o,$(SERVER_SRC)))
SERVER_HEADERS := $(shell find src/server -type f -name '*.h') $(shell find src/utils -type f -name '*.h')

PROCESS_DATA := $(SRC_DIR)/process_data/process_data_main.cc

all: build build/server-bin build/client-bin

build:
	mkdir -p build

build/server-bin: src/server_main.cc $(SERVER_OBJS)
	mkdir -p build
	# echo "$(SERVER_OBJS)"
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $@ $^ $(LDFLAGS)

build/client-bin: src/client_main.cc $(CLIENT_OBJS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $@ $^ $(LDFLAGS)

build/client-test/%.o: $(SRC_CLIENT)/%.cc $(CLIENT_HEADERS)
	mkdir -p build build/client-test
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) $(TEST_FLAGS) -c $< -o $@ $(LDFLAGS)

build/client-test-bin: src/client_main.cc $(CLIENT_TEST_OBJS)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) $(TEST_FLAGS) -o $@ $^ $(LDFLAGS)

build/client/%.o: $(SRC_CLIENT)/%.cc $(CLIENT_HEADERS)
	mkdir -p build build/client
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -c $< -o $@ $(LDFLAGS)

build/server/%.o: $(SRC_SERVER)/%.cc $(SERVER_HEADERS)
	mkdir -p build build/server
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -c $< -o $@ $(LDFLAGS)

build/query_engine/%.o: $(SRC_QUERY_ENGINE)/%.cc $(SERVER_HEADERS)
	mkdir -p build build/query_engine
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -c $< -o $@ $(LDFLAGS)

build/processor: process_data/process_data_main.cc
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXX_RELEASE_FLAGS) $^ -o $@

############################# UDP SPECIFIC BUILDS ###############################

SRC_CLIENT_UDP := $(SRC_DIR)/client_udp
SRC_SERVER_UDP := $(SRC_DIR)/server_udp

CLIENT_SRC_UDP := $(shell find src/client_udp -type f -name '*.cc')
CLIENT_OBJS_UDP := $(patsubst src/client_udp/%.cc,build/client_udp/%.o,$(CLIENT_SRC_UDP))
CLIENT_HEADERS_UDP := $(shell find src/client_udp -type f -name '*.h') $(shell find src/utils -type f -name '*.h')

SERVER_SRC_UDP := $(shell find src/server_udp -type f -name '*.cc') $(shell find src/query_engine -type f -name '*.cc')
SERVER_OBJS_UDP := $(patsubst src/server_udp/%.cc,build/server_udp/%.o,$(patsubst src/query_engine/%.cc,build/query_engine/%.o,$(SERVER_SRC_UDP)))
SERVER_HEADERS_UDP := $(shell find src/server_udp -type f -name '*.h') $(shell find src/utils -type f -name '*.h')

udp-bins: build build/server-udp-bin build/client-udp-bin

build/server-udp-bin: src/server_main_udp.cc $(SERVER_OBJS_UDP)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $@ $^ $(LDFLAGS)

build/client-udp-bin: src/client_main_udp.cc $(CLIENT_OBJS_UDP)
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -o $@ $^ $(LDFLAGS)

build/client_udp/%.o: $(SRC_CLIENT_UDP)/%.cc $(CLIENT_HEADERS_UDP)
	mkdir -p build build/client_udp
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -c $< -o $@ $(LDFLAGS)

build/server_udp/%.o: $(SRC_SERVER_UDP)/%.cc $(SERVER_HEADERS_UDP)
	mkdir -p build build/server_udp
	$(CXX) $(CXXFLAGS) $(CXX_DEBUG_FLAGS) -c $< -o $@ $(LDFLAGS)

#################################################################################

.PHONY: all clean libs data remove_spaces processed_data

# Don't add data to all as data is a PHONY target
data:
	mkdir -p data
	mkdir -p data/raw

remove_spaces:
	@for file in $(CSVS); do \
		sed -i 's/ //g' $$file; \
	done

processed_data: build/processor $(CSVS)
	mkdir -p data/processed
	@for file in $(CSVS); do \
		$< $$file; \
	done
flamegraph_setup: all
	mkdir -p external-tools/
	if [ ! -d external-tools/FlameGraph ]; then git clone https://github.com/brendangregg/FlameGraph.git external-tools/FlameGraph; fi
	chmod +x flamegraph.sh

sim_test: all flamegraph_setup
	./flamegraph.sh $(CLIENTS)

clean:
	rm -rf build

# Add any external libraries here. Make sure you put the installation location in .gitignore, and update the README.
libs:
	mkdir -p ./test/nlohmann/
	if [ ! -f ./test/nlohmann/json.hpp ]; then \
		curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp; \
	fi


################## MULTI CLIENT TEST ##################

EPP_DIR        := test/epoll-performance
EPP_SRC        := $(EPP_DIR)/antiserver.cc $(EPP_DIR)/main.cc
EPP_OBJS       := $(patsubst $(EPP_DIR)/%.cc,build/epoll-performance/%.o,$(EPP_SRC))
EPP_BIN        := build/epoll-antiserver-bin

$(EPP_BIN): $(EPP_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

build/epoll-performance/%.o: $(EPP_DIR)/%.cc $(EPP_DIR)/antiserver.h
	mkdir -p build/epoll-performance
	$(CXX) $(CXXFLAGS) -I$(EPP_DIR) -c $< -o $@

.PHONY: epoll-perf
epoll-perf: $(EPP_BIN)
	@echo "Built $(EPP_BIN)"
