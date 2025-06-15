STD_FLAG := -std=c++20
LD_FLAGS := -lspdlog -lfmt
BUILD_DIR := ../../build
OBJ_DIR := $(BUILD_DIR)/obj

all: dirs $(BUILD_DIR)/server

dirs:
	mkdir -p $(BUILD_DIR)
	mkdir -p $(OBJ_DIR)/server
	mkdir -p $(OBJ_DIR)/net
	mkdir -p $(OBJ_DIR)/query_engine

$(BUILD_DIR)/server: $(OBJ_DIR)/server/server_main.o $(OBJ_DIR)/server/server.o $(OBJ_DIR)/net/net.o $(OBJ_DIR)/server/sender.o $(OBJ_DIR)/server/receiver.o $(OBJ_DIR)/query_engine/query_engine.o
	g++ $(STD_FLAG) $(OBJ_DIR)/server/server_main.o $(OBJ_DIR)/server/server.o $(OBJ_DIR)/net/net.o $(OBJ_DIR)/server/sender.o $(OBJ_DIR)/server/receiver.o $(OBJ_DIR)/query_engine/query_engine.o -o $(BUILD_DIR)/server $(LD_FLAGS)

$(OBJ_DIR)/server/server_main.o: ../server_main.cc
	g++ $(STD_FLAG) -c ../server_main.cc -o $(OBJ_DIR)/server/server_main.o

$(OBJ_DIR)/server/server.o: server.cc
	g++ $(STD_FLAG) -c server.cc -o $(OBJ_DIR)/server/server.o

$(OBJ_DIR)/net/net.o: ../utils/net/net.cc
	g++ $(STD_FLAG) -c ../utils/net/net.cc -o $(OBJ_DIR)/net/net.o

$(OBJ_DIR)/server/receiver.o: receiver.cc
	g++ $(STD_FLAG) -c receiver.cc -o $(OBJ_DIR)/server/receiver.o

$(OBJ_DIR)/server/sender.o: sender.cc
	g++ $(STD_FLAG) -c sender.cc -o $(OBJ_DIR)/server/sender.o

$(OBJ_DIR)/query_engine/query_engine.o: ../query_engine/query_engine.cc
	g++ $(STD_FLAG) -c ../query_engine/query_engine.cc -o $(OBJ_DIR)/query_engine/query_engine.o

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
