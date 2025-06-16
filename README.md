# Trading Query Project

## Data
There's now a automatic data setup script. Call it as `bash setup_data.sh <team_id>` where team_id is 1,2,3 or 4. It will automatically download and extract the data, and will also call `make remove_spaces and `make processed_data`

It will download the files from your team's aws instance.

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

For accessing data,

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

and then run `make` to compile
