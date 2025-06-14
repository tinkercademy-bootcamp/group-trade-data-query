BUILD_DIR = build
SRC_DIR = src

all: libs server client test

server: $(BUILD_DIR)/server
	echo "THIS WONT COMPILE BECAUSE OF MISSING EXECUTOR.CC; TEST WITH A DUMMY FUNCTION INSTEAD"
	cd src/server && make && cd ../..

client: $(BUILD_DIR)/client
	cd src/client && make && cd ../..

test: 
	echo "NOT IMPLEMENTED BECAUSE CENTRAL TEST MAKEFILE ISN"T IMPLEMENTED YET"

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

# Add any external libraries you may want to use here.
# Make sure you put the installation location in .gitignore
# Update the README whenever you add a new dependency
.PHONY: libs
libs:
	mkdir -p ./test/nlohmann/
	curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp


