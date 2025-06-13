# Trading Query Project

## External Tools
run `make libs` to install nlohmann (a test dependency)

## Dir Structure
The following is the current directory structure of the directory of the repo

```
.
├── build
├── .gitignore
├── Makefile
├── README.md
├── src
│   ├── client
│   │   ├── client.cc
│   │   ├── client.h
│   │   └── Makefile
│   ├── Makefile
│   ├── server
│   │   ├── Makefile
│   │   ├── receiver.cc
│   │   ├── sender.cc
│   │   ├── sender.h
│   │   ├── server.cc
│   │   └── server.h
|   ├── executor
|   │   ├── executor.h
|   |   └── executor.cc
│   └── utils
│       ├── helper
│       │   └── utils.h
│       ├── net
│       │   ├── net.cc
│       │   └── net.h
│       └── query.h
└── test
    ├── correctness
    │   └── Makefile
    |   └── test-client.cpp
    |   └── dummy-client.cpp
    |   └── basic-tests.json
    ├── Makefile
    ├── performance
    │   └── Makefile
    └── README.md
```