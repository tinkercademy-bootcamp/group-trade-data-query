STD_FLAG := -std=c++20
LD_FLAGS := -lspdlog -lfmt
BUILD_DIR := ../../build
OBJ_DIR := $(BUILD_DIR)/obj

all: dirs $(BUILD_DIR)/server

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)/server
	mkdir -p $(OBJ_DIR)/net

$(BUILD_DIR)/server: $(OBJ_DIR)/server/server_main.o $(OBJ_DIR)/server/server.o $(OBJ_DIR)/net/net.o
	g++ $(STD_FLAG) $(OBJ_DIR)/server/server_main.o $(OBJ_DIR)/server/server.o $(OBJ_DIR)/net/net.o -o $(BUILD_DIR)/server $(LD_FLAGS)

$(OBJ_DIR)/server/server_main.o: ../server_main.cc
	g++ $(STD_FLAG) -c ../server_main.cc -o $(OBJ_DIR)/server/server_main.o

$(OBJ_DIR)/server/server.o: server.cc
	g++ $(STD_FLAG) -c server.cc -o $(OBJ_DIR)/server/server.o

$(OBJ_DIR)/net/net.o: ../utils/net/net.cc
	g++ $(STD_FLAG) -c ../utils/net/net.cc -o $(OBJ_DIR)/net/net.o

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

.PHONY: remake
remake:
	make clean && make