all: libs server client test data

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

server: build/server
build/server:
	echo "THIS WONT COMPILE BECAUSE OF MISSING EXECUTOR.CC; TEST WITH A DUMMY FUNCTION INSTEAD \(You can use early return in line 82 of server\)"
	cd src/server && make && cd ../..

client: build/client
build/client:
	cd src/client && make && cd ../..

test: 
	echo "NOT IMPLEMENTED BECAUSE CENTRAL TEST MAKEFILE ISN'T IMPLEMENTED YET"

.PHONY: clean
clean:
	rm -rf build
