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
## Performance tests for Memory Accesses

To run the tests on database accesses, one can run the `test-executor.cc` which accesses every page at once and tracks the physical memory allocated to the process.

To compile the `test-executor.cc`,

```bash
mkdir -p build/test-db

g++ test/performance/db-tests/test-executor.cc src/query_engine/query_engine.cc -o build/test-db

```

Then run the binary with sudo and an argument of number of pages to access.

```bash
sudo ./build/test-db <number-of-pages>
```

To view the plot on ssh servers, as we don't have a terminal display set up,
run the following commands
```bash
gnuplot

set terminal png size 1000,600 #depends on what size you want

set output "/path/to/times.png"

plot "~/group-trade-data-query/test/performance/db-tests/output/times.txt" using 1:2 with linespoints title "Time Latencies" #for plotting text.txt

set output "/path/to/differences.png"

plot "~/group-trade-data-query/test/performance/db-tests/output/differences.txt" using 1:2 with linespoints title "Time Latencies" #for plotting differences.txt

```
Now you may find the plots in the folder you wanted them in.