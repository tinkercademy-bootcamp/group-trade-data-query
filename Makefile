all: libs server client test

server: build/server
build/server:
	echo "THIS WONT COMPILE BECAUSE OF MISSING EXECUTOR.CC; TEST WITH A DUMMY FUNCTION INSTEAD \(You can use early return in line 82 of server\)"
	cd src/server && make && cd ../..

client: build/client
build/client:
	cd src/client && make && cd ../..

test: 
	echo "NOT IMPLEMENTED BECAUSE CENTRAL TEST MAKEFILE ISN"T IMPLEMENTED YET"

.PHONY: clean
clean:
	rm -rf build

# Add any external libraries you may want to use here. Make sure you put the installation location in .gitignore, and update the README whenever you add a new dependency
.PHONY: libs
libs:
	mkdir -p ./test/nlohmann/
	curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp