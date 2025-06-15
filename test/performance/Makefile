# all: database-tests

# database-tests: 


.PHONY: all test clean server client run-tests smoke-test memory-test help
.DEFAULT_GOAL := all

# Directories
ROOT_DIR := ../..
SRC_DIR := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
SERVER_BUILD := $(BUILD_DIR)
CLIENT_BUILD := $(BUILD_DIR)
TEST_BUILD := $(BUILD_DIR)

# Source paths
SERVER_SRC := $(SRC_DIR)/server
CLIENT_SRC := $(SRC_DIR)/client

# Executables
SERVER_EXEC := $(SERVER_BUILD)/server
CLIENT_EXEC := $(CLIENT_BUILD)/client
TEST_CLIENT := $(TEST_BUILD)/test_client
DUMMY_CLIENT := dummy_client

# Compiler settings
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -std=c11 -g -DDEBUG
CXXFLAGS := -Wall -Wextra -std=c++17 -g -DDEBUG
LDFLAGS := -pthread

# Test configuration
SERVER_PORT := 8080
TEST_DATA_DIR := ./test_data
MAX_CLIENTS := 10
TEST_TIMEOUT := 30

# Colors
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[1;33m
BLUE := \033[0;34m
NC := \033[0m

# all: setup server client test-client run-tests
# all: test-client run-tests
all: run-dummy-tests

run-dummy-tests: dummy-client test-client run-tests

setup: ## Create test directories and files
	@echo "$(YELLOW)Setting up correctness test environment...$(NC)"
	@mkdir -p $(SERVER_BUILD) $(CLIENT_BUILD) $(TEST_BUILD) $(TEST_DATA_DIR)
	@if [ ! -f $(SERVER_EXEC) ]; then \
		echo "$(YELLOW)Server executable not found. Building...$(NC)"; \
		$(MAKE) server || (echo "$(RED)✗ Server build failed$(NC)" && exit 1); \
	else \
		echo "$(GREEN)✓ Server already built$(NC)"; \
	fi
	@if [ ! -f $(CLIENT_EXEC) ]; then \
		echo "$(YELLOW)Client executable not found. Building...$(NC)"; \
		$(MAKE) client || (echo "$(RED)✗ Client build failed$(NC)" && exit 1); \
	else \
		echo "$(GREEN)✓ Client already built$(NC)"; \
	fi
	@echo "$(GREEN)✓ Test environment ready$(NC)"


server: ## Build server for testing
	@echo "$(YELLOW)Building server via server's Makefile...$(NC)"
	@$(MAKE) -C $(SERVER_SRC) BUILD_DIR=$(SERVER_BUILD)

client: ## Build client for testing
	@echo "$(YELLOW)Building client via client's Makefile...$(NC)"
	@$(MAKE) -C $(CLIENT_SRC) BUILD_DIR=$(CLIENT_BUILD)

dummy-client:  ## Build the C++ multithreaded test driver
	@echo "$(YELLOW)Building dummy client driver...$(NC)"
	$(CXX) $(CXXFLAGS) -o $(DUMMY_CLIENT) dummy_client.cpp $(LDFLAGS)

perf_test: ## Build the C++ multithreaded test driver
	@echo "$(YELLOW)Building test client driver...$(NC)"
	$(CXX) $(CXXFLAGS) -o $(TEST_CLIENT) perf_test.cpp $(LDFLAGS)

run_tests: test-client ## Run tests using the test driver
	@echo "$(BLUE)Running tests...$(NC)"
	@$(TEST_CLIENT)
