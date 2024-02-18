# Log-Structured-KV-Storage-Engine

* Implemented a Log-Structured Key-Value Storage Engine in C++, drawing inspiration from databases like Bitcask, LevelDB, RocskDB, etc.

## About
* Lightning-fast writes by appending data directly to a binary file, optimizing write performance.
* Reads are highly efficient, leveraging an in-memory index to store byte offsets of the latest records in the data file. 
* It is fault-tolerant, persisting both data and index to the file system.
* Supports crash recovery through "write-ahead logging," allowing the reconstruction of the index in case of system crashes during the persistence process.
* Data is stored across multiple immutable files + one active/mutable file for handling large data. 

## TODO
* Write a compaction algorithm (running in background) to merge older chunks and their indexes. (Since data file is append only, a key can be present in multiple files but its latest record will be in one of the latest files.)
* Implement LSM tree.

## How to run
* Run executable file (engine.exe) in build folder

## References
* https://riak.com/assets/bitcask-intro.pdf
