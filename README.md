# Trading Query Project

## Makefiles
Each component has its own makefile and can be compiled by running `make` in the respective directory. To make the whole program, call `make` from the project root.

### Extending the Makefiles
If your making a new main process, make a seperate Makefile.
If you're adding components to an executable, make a new rule for the component's object file and add it as a pre-req for the target executable. Make sure to add any required directories in the `make dirs` target.

In general, just read the Makefile and you'll immediately know what to do.

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
│   ├── client
│   │   ├── client.cc
│   │   └── client.h
│   ├── client_main.cc
│   ├── executor
│   │   ├── executor.cc
│   │   └── executor.h
│   ├── server
│   │   ├── receiver.cc
│   │   ├── sender.cc
│   │   ├── sender.h
│   │   ├── server.cc
│   │   └── server.h
│   ├── server_main.cc
│   └── utils
│       ├── helper
│       │   └── utils.h
│       ├── net
│       │   └── net.h
│       └── query.h
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
