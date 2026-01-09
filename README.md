# E-Store Simulation

A multithreaded C++ simulation of an online store.
The project focuses on concurrency, synchronization, and systems-level design.

## Features
- Thread-safe task queue
- Producer/consumer model
- Worker threads using pthreads
- Modular C++ structure

## Build
Requirements: g++, make, Linux or WSL

Build the project:
make

The executable is created at:
build/estoresim

## Run
Normal mode:
make run-sim

Detailed mode:
make run-sim-fine

## Notes
Some systems may require elevated permissions.
If needed:
sudo make run-sim

## Clean
Remove build files:
make clean
