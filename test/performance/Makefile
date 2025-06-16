.PHONY: all test clean run-tests perf-test help
.DEFAULT_GOAL := all

# Directories
ROOT_DIR := ../..
SRC_DIR := $(ROOT_DIR)/src
BUILD_DIR := $(ROOT_DIR)/build
SERVER_BUILD := $(BUILD_DIR)
CLIENT_BUILD := $(BUILD_DIR)
TEST_BUILD := $(BUILD_DIR)
MULTI_CLIENT_BUILD := $(BUILD_DIR)

# Executables
SERVER_EXEC := $(SERVER_BUILD)/server-bin
CLIENT_EXEC := $(CLIENT_BUILD)/client-test-bin
SIMPLE_TEST := $(TEST_BUILD)/simple_test
DUMMY_CLIENT := $(TEST_BUILD)/dummy_client
MULTI_CLIENT := $(MULTI_CLIENT_BUILD)/multi_client
PERF_TEST := $(TEST_BUILD)/perf_test

# Compiler settings
CC := gcc
CXX := g++
CFLAGS := -Wall -Wextra -std=c11 -g -DDEBUG
CXXFLAGS := -Wall -Wextra -std=c++20 -g -DDEBUG -O0 -fsanitize=address -pedantic
LDFLAGS := -pthread

# Test configuration
SERVER_PORT := 8080
TEST_TIMEOUT := 30
DEFAULT_CLIENTS := 5

# Colors
RED := \033[0;31m
GREEN := \033[0;32m
YELLOW := \033[1;33m
BLUE := \033[0;34m
CYAN := \033[0;36m
NC := \033[0m

# Help target
help: ## Show available targets
	@echo "$(CYAN)Available targets:$(NC)"
	@echo "  $(GREEN)all$(NC)                   - Build everything and run basic performance test"
	@echo "  $(GREEN)setup$(NC)                 - Create test directories and build server/client if needed"
	@echo "  $(GREEN)server$(NC)                - Build server executable"
	@echo "  $(GREEN)client$(NC)                - Build client executable"
	@echo "  $(GREEN)dummy-client$(NC)          - Build dummy client for testing"
	@echo "  $(GREEN)perf-test$(NC)             - Build performance test driver"
	@echo "  $(GREEN)run-tests$(NC)             - Run basic functionality tests (dummy client + simple test)"
	@echo "  $(GREEN)run-perf-tests$(NC)        - Run performance tests with options"
	@echo "  $(GREEN)perf-test-real$(NC)        - Run performance tests with real client"
	@echo "  $(GREEN)benchmark-comparison$(NC)  - Compare dummy client perf across multiple client counts"
	@echo "  $(GREEN)perf-monitor$(NC)          - Continuously monitor performance every TEST_TIMEOUT seconds"
	@echo "  $(GREEN)analyze-results$(NC)       - Analyze performance_results.csv"
	@echo "  $(GREEN)clean$(NC)                 - Clean build artifacts"
	@echo "  $(GREEN)clean-results$(NC)         - Clean logs and result CSVs"
	@echo "  $(GREEN)clean-all$(NC)             - Clean everything"
	@echo "  $(GREEN)check-deps$(NC)            - Check if all required executables are built"
	@echo "  $(GREEN)show-config$(NC)           - Show current test configuration"
	@echo ""
	@echo "$(CYAN)Examples:$(NC)"
	@echo "  make run-perf-tests                     # Run test with default client and 5 clients"
	@echo "  make run-perf-tests CLIENTS=20          # Run test with 20 clients"


all:
	$(MAKE) run-perf-tests 

run-perf-tests: setup dummy-client perf-test ## Run performance tests with configurable number of clients
	@$(eval CLIENTS := $(or $(CLIENTS),$(DEFAULT_CLIENTS)))
	@$(eval EXEC := $(or $(EXEC),$(if $(USE_DUMMY),$(DUMMY_CLIENT),$(CLIENT_EXEC))))
	@echo "$(BLUE)Running performance tests with $(CLIENTS) concurrent clients...$(NC)"
	@echo "$(BLUE)Using executable: $(EXEC)$(NC)"
	@if [ ! -f "$(EXEC)" ]; then \
		echo "$(RED)✗ Executable not found: $(EXEC)$(NC)"; \
		echo "$(YELLOW)Available executables:$(NC)"; \
		ls -la $(BUILD_DIR)/ 2>/dev/null || echo "No executables found"; \
		exit 1; \
	fi
	@echo "$(CYAN)Starting performance test...$(NC)"
	@$(PERF_TEST) --clients $(CLIENTS) --exec $(EXEC) --tests ./performance_tests.json
	@echo "$(GREEN)✓ Performance test completed$(NC)"

run-incremental-tests: setup dummy-client perf-test ## Run perf tests incrementally until failure
	@$(eval EXEC := $(or $(EXEC),$(if $(USE_DUMMY),$(DUMMY_CLIENT),$(CLIENT_EXEC))))
	@$(eval MAX_CLIENTS := $(or $(MAX_CLIENTS),100))
	@$(eval INCREMENT := $(or $(INCREMENT),5))
	@echo "$(CYAN)Starting incremental load test...$(NC)"
	@CLIENTS=1; \
	while [ $$CLIENTS -le $(MAX_CLIENTS) ]; do \
		echo "$(BLUE)Testing with $$CLIENTS clients...$(NC)"; \
		$(PERF_TEST) --clients $$CLIENTS --exec $(EXEC) --tests ./performance_tests.json > perf_tmp.csv 2>&1; \
		if tail -n +2 perf_tmp.csv | grep -E '^Client_.*false'; then \
			echo "$(RED)✗ Test failed with $$CLIENTS clients. Stopping.$(NC)"; \
			break; \
		else \
			echo "$(GREEN)✓ Passed with $$CLIENTS clients$(NC)"; \
		fi; \
		tail -n +2 perf_tmp.csv | grep -E '^Client_' >> performance_results.csv; \
		CLIENTS=$$((CLIENTS + $(INCREMENT))); \
	done; \
	rm -f perf_tmp.csv; \
	echo "$(CYAN)✓ Incremental testing completed.$(NC)"



setup: server client
	@echo "$(YELLOW)Setting up performance test environment...$(NC)"
	@mkdir -p $(SERVER_BUILD) $(CLIENT_BUILD) $(TEST_BUILD)
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
	@$(MAKE) -C $(ROOT_DIR) build/server-bin

client: ## Build client for testing
	@echo "$(YELLOW)Building client via client's Makefile...$(NC)"
	@$(MAKE) -C $(ROOT_DIR) -B build/client-test-bin

dummy-client: ## Build the dummy client for testing
	@echo "$(YELLOW)Building dummy client...$(NC)"
	$(CXX) $(CXXFLAGS) -o $(DUMMY_CLIENT) dummy_client.cpp $(LDFLAGS)

perf-test: ## Build the performance test client
	@echo "$(YELLOW)Building performance test client...$(NC)"
	$(CXX) $(CXXFLAGS) -o $(PERF_TEST) perf_test.cpp $(LDFLAGS)

# Benchmarking targets
benchmark-comparison: perf-test dummy-client ## Compare performance across different client counts
	@echo "$(CYAN)Running benchmark comparison across different client counts...$(NC)"
	@echo "$(YELLOW)1 client:$(NC)"
	@$(PERF_TEST) --clients 1 --exec $(DUMMY_CLIENT) --tests ./performance_tests.json > benchmark_1_client.log 2>&1
	@echo "$(YELLOW)5 clients:$(NC)"
	@$(PERF_TEST) --clients 5 --exec $(DUMMY_CLIENT) --tests ./performance_tests.json > benchmark_5_clients.log 2>&1
	@echo "$(YELLOW)10 clients:$(NC)"
	@$(PERF_TEST) --clients 10 --exec $(DUMMY_CLIENT) --tests ./performance_tests.json > benchmark_10_clients.log 2>&1
	@echo "$(YELLOW)20 clients:$(NC)"
	@$(PERF_TEST) --clients 20 --exec $(DUMMY_CLIENT) --tests ./performance_tests.json > benchmark_20_clients.log 2>&1
	@echo "$(GREEN)✓ Benchmark comparison completed. Check benchmark_*_client*.log files$(NC)"

# Continuous performance monitoring
perf-monitor: perf-test ## Run continuous performance monitoring (every TEST_TIMEOUT seconds)
	@echo "$(CYAN)Starting continuous performance monitoring...$(NC)"
	@echo "$(YELLOW)Press Ctrl+C to stop$(NC)"
	@while true; do \
		echo "$(BLUE)Running performance check at $$(date)...$(NC)"; \
		$(PERF_TEST) --clients 5 --exec $(DUMMY_CLIENT) --tests ./performance_tests.json | tail -10; \
		sleep TEST_TIMEOUT; \
	done

# Results analysis
analyze-results: ## Analyze CSV results (requires performance_results.csv)
	@if [ -f "performance_results.csv" ]; then \
		echo "$(CYAN)Analyzing performance results...$(NC)"; \
		total=$$(tail -n +2 performance_results.csv | wc -l); \
		passed=$$(tail -n +2 performance_results.csv | grep ',true,' | wc -l); \
		failed=$$(tail -n +2 performance_results.csv | grep ',false,' | wc -l); \
		avg_time=$$(tail -n +2 performance_results.csv | cut -d',' -f4 | tr -d '"' | awk '{sum+=$$1; count++} END {if (count > 0) printf "%.2f", sum/count; else print "0.00"}'); \
		unique_clients=$$(tail -n +2 performance_results.csv | cut -d',' -f1 | sort -u | wc -l); \
		echo "$(YELLOW)Total test runs:$(NC) $$total"; \
		echo "$(YELLOW)Passed tests:$(NC) $$passed"; \
		echo "$(YELLOW)Failed tests:$(NC) $$failed"; \
		echo "$(YELLOW)Average response time:$(NC) $$avg_time ms"; \
		echo "$(YELLOW)Unique clients:$(NC) $$unique_clients"; \
		echo "$(GREEN)✓ Analysis complete. Full results in performance_results.csv$(NC)"; \
	else \
		echo "$(RED)✗ No performance_results.csv found. Run a performance test first.$(NC)"; \
	fi

# Clean targets
clean: ## Clean build artifacts
	@echo "$(YELLOW)Cleaning build artifacts...$(NC)"
	@rm -f $(SIMPLE_TEST) $(DUMMY_CLIENT) $(PERF_TEST)
	@rm -f *.o *.so *.a
	@echo "$(GREEN)✓ Build artifacts cleaned$(NC)"

clean-results: ## Clean test results and logs
	@echo "$(YELLOW)Cleaning test results and logs...$(NC)"
	@rm -f performance_results.csv
	@rm -f benchmark_*.log
	@rm -f *.log
	@echo "$(GREEN)✓ Results and logs cleaned$(NC)"

clean-all: clean clean-results ## Clean everything
	@echo "$(GREEN)✓ Everything cleaned$(NC)"

# Check if executables exist
check-deps: ## Check if all dependencies are built
	@echo "$(CYAN)Checking dependencies...$(NC)"
	@echo -n "Server executable: "; \
	if [ -f "$(SERVER_EXEC)" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi
	@echo -n "Client executable: "; \
	if [ -f "$(CLIENT_EXEC)" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi
	@echo -n "Dummy client: "; \
	if [ -f "$(DUMMY_CLIENT)" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi
	@echo -n "Test client: "; \
	if [ -f "$(SIMPLE_TEST)" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi
	@echo -n "Performance client: "; \
	if [ -f "$(PERF_TEST)" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi
	@echo -n "Test file: "; \
	if [ -f "./performance_tests.json" ]; then echo "$(GREEN)✓ Found$(NC)"; else echo "$(RED)✗ Missing$(NC)"; fi

show-config: ## Show current test configuration
	@echo "$(CYAN)Performance Test Configuration:$(NC)"
	@echo "  Default Clients: $(DEFAULT_CLIENTS)"
	@echo "  Test Timeout: $(TEST_TIMEOUT)s"
	@echo "  Server Port: $(SERVER_PORT)"
	@echo "  Build Directory: $(BUILD_DIR)"
	@echo ""
	@echo "$(CYAN)Available Executables:$(NC)"
	@ls -la $(BUILD_DIR)/ 2>/dev/null | grep -E '\.(exe|out)$|client$|server$' || echo "  No executables found"

multi-client: 
	@echo "$(YELLOW)Building multi-client via the Makefile...$(NC)"
	$(CXX) $(CXXFLAGS) -o $(MULTI_CLIENT) multi_client/multi_client_main.cc multi_client/multi_client.cc $(LDFLAGS) -lfmt

run-distributed-tests:
	bash distributed_perf_test.sh
