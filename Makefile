.PHONY: server client clean libs

all: libs server client test

server:
	cd src/server && make && cd ../..

client:
	cd src/client && make && cd ../..	

test: 
	echo "NOT IMPLEMENTED BECAUSE CENTRAL TEST MAKEFILE ISN'T IMPLEMENTED YET"

clean:
	rm -rf build

# Add any external libraries here. Make sure you put the installation location in .gitignore, and update the README.
libs:
	mkdir -p ./test/nlohmann/
	if [ ! -f ./test/nlohmann/json.hpp ]; then \
		curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp; \
	fi
