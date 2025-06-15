# Trading Query Project

## Compiling the Project
For accessing data, run the following command:

```bash
make data
```
Then copy the tar.gz files into `data/raw` and run

```bash
tar -xvzf trades-<file>.tar.gz
```

to untar the files and run

```bash
make remove_spaces  
make processed_data
```

and then run `make all` to compile.

**To run the server:** `./build/server-bin`

**To run the client:** `./build/client-bin`

## Data
Additionally, there's now a automatic data setup script. Call it as `bash setup_data.sh <team_id>` where team_id is 1,2,3 or 4. It will automatically download and extract the data, and will also call `make remove_spaces` and `make processed_data`

It will download the files from your team's aws instance.

## External Tools
Run `make libs` to install nlohmann, used as a test dependency.

## Directory Structure
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
        └── Makefile
```
