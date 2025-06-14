# Trading Query Project

## External Tools
run `make libs` to install nlohmann (a test dependency)

## Dir Structure
The following is the current directory structure of the directory of the repo

```
.
├── .gitignore
├── build
│   └── processor
├── data
│   ├── processed
│   │   ├── trades-example.bin
│   │   ├── trades-small.bin
│   │   └── trades-tiny.bin
│   └── raw
│       ├── trades-example.csv
│       ├── trades-small.csv
│       └── trades-tiny.csv
├── Makefile
├── process_data
│   └── process_data_main.cc
├── README.md
├── requirements.txt
├── src
│   ├── client
│   │   ├── client.cc
│   │   ├── client.h
│   │   └── Makefile
│   ├── executor
│   │   ├── executor.cc
│   │   └── executor.h
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
    │   ├── basic-tests.json
    │   ├── db-tests
    │   │   ├── db.ipynb
    │   │   ├── db-testcases
    │   │   └── db_testcases.py
    │   ├── dummy-client.cpp
    │   ├── Makefile
    │   └── test-client.cpp
    ├── Makefile
    ├── performance
    │   └── Makefile
    └── README.md
```