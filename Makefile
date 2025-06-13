.PHONY: libs
libs:
	mkdir -p test/nlohmann
	curl -sL https://raw.githubusercontent.com/nlohmann/json/develop/single_include/nlohmann/json.hpp -o ./test/nlohmann/json.hpp