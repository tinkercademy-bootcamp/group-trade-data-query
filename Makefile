.PHONY: libs data

CSVS := $(wildcard data/raw/*.csv)

libs:
	mkdir -p test/nlohmann
	curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp

build/processor: process_data/process_data_main.cc
	mkdir -p build
	g++ -std=c++20 $^ -o $@

# Don't add data to all as data is a PHONY target
data: $(CSVS) build/processor
	mkdir -p data/processed
	@for file in $(CSVS); do \
		./build/processor $$file; \
	done
