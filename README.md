# Trading Query Project

## Compiling and Running
Firstly, make sure you have the data downloaded and extracted in the `data/raw` directory. If you haven't done this yet, refer to the [Data](#data) section below.
To compile the project, run:

```bash
make # for the default build (TCP)
make udp-bins # for the UDP build
```
The TCP binaries are named `server-bin` and `client-bin`, while the UDP binaries are named `server-udp-bin` and `client-udp-bin`. All of these are in the build/ folder.

## Data
There's now a automatic data setup script. Call it as `bash setup_data.sh <team_id>` where team_id is 1,2,3 or 4. It will automatically download and extract the data, and will also call `make remove_spaces and `make processed_data`

It will download the files from your team's aws instance.

Or, to it manually:

Run

```bash
make data
```
then copy the tar.gz files into `data/raw` and run

```bash
tar -xvzf trades-<file>.tar.gz
```

to untar the files and run

```bash
make remove_spaces  
make processed_data
```
After flamegraph setup, run
```
make sim_test CLIENTS=x
```
whose default is 10 clients. 

## External Tools
run `make libs` to install nlohmann (a test dependency)

run `make flamegraph_setup` to install flamegraph.

## Tests
For running the tests, firstly ensure that you have your data in place, and it has been processed properly, and you are able to compile your server and client using make command.

After compiling the dependencies, run the server on one of your terminals. Use ```./build/server-bin``` from root directory.

Go to the folder: ```test/performance``` and run the ```make``` command. It will ensure that all the dependencies have been already made and make the dependencies for client in test mode. If some of the dependencies are not present or outdated, it will update it automatically.

It will run the tests and display the results accordingly. 

```make run-perf-tests```   # Run test with default client and 5 clients

```make run-perf-tests CLIENTS=20```   # Run test with 20 clients

To change the tests, Just edit the ```performance_tests.json```, it has the fields for input and expected output. Just fill up the fields and your test will be added automatically!

## How does testing work?
- The testing framework spawns clients with relevant commands that have been fetched from ```performance_tests.json```. The clients are currently spawned using popen command in read mode. So, the clients work in single-shot mode (using the flag -DTESTMODE). This ensures that the clients take in input and output in a single flush, whereas ensuring that the clients do not print any spdlog output as well. Then we hear on the clients side, and match the expected output against the output received.

## How to make changes?
- For the current purposes, most of the changes can be accommodated just by editing the ```performance_tests.json``` inside performance folder. Fot the rest changes, you may edit the perf_test.cpp or the Makefile in perfromance folder.

- For running tests across servers, you would have to change the server-ip address and port number in ```src/client-main.cc```. and change the port number in ```src/server-main.cc```.
After that, launch the server-bin on one aws instance and client-bin on another aws instance.

## Dir Structure
The following is the current directory structure of the repository:

```
 > tree       
.
├── build (GITIGNORE)
├── data (GITIGNORE)
├── flamegraph.sh
├── Makefile
├── process_data
│   ├── check.cc
│   └── process_data_main.cc
├── README.md
├── requirements.txt
├── setup_data.sh
├── src
│   ├── client
│   │   ├── client.cc
│   │   └── client.h
│   ├── client_main.cc
│   ├── client_main_udp.cc
│   ├── client_udp
│   │   ├── client_udp.cc
│   │   └── client_udp.h
│   ├── query_engine
│   │   ├── query_engine.cc
│   │   └── query_engine.h
│   ├── server
│   │   ├── receiver.cc
│   │   ├── sender.cc
│   │   ├── sender.h
│   │   ├── server.cc
│   │   └── server.h
│   ├── server_main.cc
│   ├── server_main_udp.cc
│   ├── server_udp
│   │   ├── sender_udp.cc
│   │   ├── sender_udp.h
│   │   ├── server_udp.cc
│   │   └── server_udp.h
│   └── utils
│       ├── helper
│       │   └── utils.h
│       ├── net
│       │   └── net.h
│       ├── net_udp
│       │   └── net_udp.h
│       └── query.h
└── test
    ├── correctness
    │   ├── basic-tests.json
    │   ├── db-tests
    │   │   ├── db.ipynb
    │   │   └── db_testcases.py
    │   ├── dummy-client.cpp
    │   ├── Makefile
    │   └── test-client.cpp
    ├── Makefile
    ├── performance
    │   ├── dummy_client.cpp
    │   ├── Makefile
    │   ├── multi_client
    │   │   ├── multi_client.cc
    │   │   ├── multi_client.h
    │   │   └── multi_client_main.cc
    │   ├── performance_results.csv
    │   ├── performance_tests.json
    │   ├── perf_test.cpp
    │   └── README.md
    └── README.md
```
