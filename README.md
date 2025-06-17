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
make smalltest 
make bigtest
```

## External Tools
run `make libs` to install nlohmann (a test dependency)

run `make flamegraph_setup` to install flamegraph.

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
