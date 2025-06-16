# Trading Query Project


## Compiling and Running
Firstly, make sure you have the data downloaded and extracted in the `data/raw` directory. If you haven't done this yet, refer to the [Data](#data) section below.
To compile the project, run:

```bash
make # for the default build (TCP)
make udp-bins # for the UDP build
```
The TCP binaries are named `server-bin` and `client-bin`, while the UDP binaries are named `server-udp-bin` and `client-udp-bin`.

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

## External Tools
run `make libs` to install nlohmann (a test dependency)

## Dir Structure
The following is the current directory structure of the directory of the repo## Directory Structure
The following is the current directory structure of the repository.

```
.
├── .gitignore
├── Makefile
├── README.md
├── requirements.txt
├── setup_data.sh
├── build (not included in the repository)
├── data (not included in the repository)
├── process_data
│   ├── check.cc
│   └── process_data_main.cc
├── src
│   ├── client_main.cc
│   ├── server_main.cc
│   ├── client
│   │   ├── client.cc
│   │   └── client.h
│   ├── query_engine
│   │   ├── query_engine.cc
│   │   └── query_engine.h
│   ├── server
│   │   ├── receiver.cc
│   │   ├── sender.cc
│   │   ├── sender.h
│   │   ├── server.cc
│   │   └── server.h
│   └── utils
│       ├── query.h
│       ├── helper
│       │   └── utils.h
│       └── net
│           ├── net.cc
│           └── net.h
└── test
    ├── Makefile
    ├── README.md
    ├── correctness
    │   ├── Makefile
    │   ├── basic-tests.json
    │   ├── dummy-client.cpp
    │   ├── test-client.cpp
    │   └── db-tests
    │       ├── db.ipynb
    │       └── db_testcases.py
    └── performance
        └── multi_client
            └── multi_client_main.cc
            └── multi_client.cc
            └── multi_client.h
        └── Makefile
        └── dummy_client.cpp
        └── perf_test.cpp
        └── performance_tests.json
        └── README.md
```
