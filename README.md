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
The following is the current directory structure of the directory of the repo

```
.
├── .gitignore
├── build
│   ├── client
│   │   └── client.o
│   ├── client-bin
│   ├── processor
│   ├── server
│   │   ├── receiver.o
│   │   ├── sender.o
│   │   └── server.o
│   └── server-bin
├── data
│   ├── processed
│   │   └── trades-example.bin
│   └── raw
│       └── trades-example.csv
├── Makefile
├── process_data
│   └── process_data_main.cc
├── README.md
├── requirements.txt
├── src
│   ├── client
│   │   ├── client.cc
│   │   ├── client.h
│   │   └── Makefile
│   ├── query_engine
│   │   ├── query_engine.cc
│   │   └── query_engine.h
│   ├── Makefile
│   ├── server
│   │   ├── Makefile
│   │   ├── receiver.cc
│   │   ├── sender.cc
│   │   ├── sender.h
│   │   ├── server.cc
│   │   └── server.h
│   ├── client_main.cc
│   ├── server-main.cc
│   └── utils
│       ├── helper
│       │   └── utils.h
│       ├── net
│       │   ├── net.cc
│       │   └── net.h
│       └── query.h
└── test
    ├── correctness
    │   ├── basic-tests.json
    │   ├── db-tests
    │   │   ├── db.ipynb
    │   │   └── db_testcases.py
    │   ├── dummy-client.cpp
    │   ├── Makefile
    │   └── test-client.cpp
    ├── nlohmann
    │   └── json.hpp
    ├── performance
    │   └── Makefile
    └── README.md
```
