For running any test, goto home folder and run: 
make libs
This will install nlohmann::Json that is used for loading tests.
For performance tests, just change to performance folder and run
make
This will spawn clients and return the result about how many tests passed.
Note: Ensure that you have server already running before running tests.
Use make help to know more about tests in performance folder.


## Performance tests for Memory Accesses

To run the tests on database accesses, one can run the `test-executor.cc` which accesses every page at once and tracks the physical memory allocated to the process.

To compile the `test-executor.cc`,

```bash
g++ test/performance/db-tests/test-executor.cc src/query_engine/query_engine.cc -o build/test-db
```

Then run the binary with sudo and an argument of number of pages to access.

```bash
sudo ./build/test-db <number-of-pages>
```

Then one can plot the data using 
```bash
gnuplot
> plot "times.txt" using 1:2 with linespoints title "Time Latencies"
> plot "differences.txt" using 1:2 with linespoints title "Physical Memory read from the file"
```