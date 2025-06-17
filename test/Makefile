.PHONY: all run-perf-tests analyze-results clean clean-all

PERF_DIR := performance
PERF_MAKE := $(MAKE) -C $(PERF_DIR)

all: run-perf-tests

run-perf-tests:
	$(PERF_MAKE) run-perf-tests

analyze-results:
	$(PERF_MAKE) analyze-results

clean:
	$(PERF_MAKE) clean

clean-all:
	$(PERF_MAKE) clean-all
