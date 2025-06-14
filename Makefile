BUILD_DIR = build
SRC_DIR = src

all: libs server client test

server:

client:

test:

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)


# Add any external libraries you may want to use here.
# Make sure you put the installation location in .gitignore
# Update the README whenever you add a new dependency
.PHONY: libs
libs:
	curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp


