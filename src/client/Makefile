STD_FLAG := -std=c++20
LD_FLAGS := -lspdlog -lfmt
BUILD_DIR := ../../build
OBJ_DIR := $(BUILD_DIR)/obj

all: dirs $(BUILD_DIR)/client

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)/client
	mkdir -p $(OBJ_DIR)/net

$(BUILD_DIR)/client: $(OBJ_DIR)/client/client_main.o $(OBJ_DIR)/client/client.o $(OBJ_DIR)/net/net.o
	g++ $(STD_FLAG) $(OBJ_DIR)/client/client_main.o $(OBJ_DIR)/client/client.o $(OBJ_DIR)/net/net.o -o $(BUILD_DIR)/client $(LD_FLAGS)

$(OBJ_DIR)/client/client_main.o: ../client_main.cc
	g++ $(STD_FLAG) -c ../client_main.cc -o $(OBJ_DIR)/client/client_main.o

$(OBJ_DIR)/client/client.o: client.cc
	g++ $(STD_FLAG) -c client.cc -o $(OBJ_DIR)/client/client.o

$(OBJ_DIR)/net/net.o: ../utils/net/net.cc
	g++ $(STD_FLAG) -c ../utils/net/net.cc -o $(OBJ_DIR)/net/net.o

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: remake
remake:
	make clean && make