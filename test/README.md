For running any test, goto home folder and run: 
make libs
This will install nlohmann::Json that is used for loading tests.
For performance tests, just change to performance folder and run
make
This will spawn clients and return the result about how many tests passed.
Note: Ensure that you have server already running before running tests.
Use make help to know more about tests in performance folder.
