# Network Simulator

A SystemC-based network simulation framework.

## Overview

This project implements a network simulator for modeling and analyzing network hardware, including Network Interface Card (NIC) and Switch.

## Project Structure

```
network-simulator/
├── include/            # Header directory
├── src/                # Source code directory
├── test/               # Test code directory
├── Makefile            # Makefile
└── README.md
```

## Prerequisites

### System Requirements
- Linux
- C++ compiler with C++20 support or later:

### SystemC Installation
- SystemC 3.0.1 or later
- Download from: https://github.com/accellera-official/systemc
- Follow the SystemC installation guide for your platform

## Building

### Configuration
Before building, you need to modify the `Makefile` to set the correct paths:

1. Set `SYSTEMC_HOME` to your SystemC installation path
2. Set `NIC_SIMULATOR_HOME` to your network-simulator project path
3. Set `TEST_NAME` to the test you want to run

```makefile
# Example configuration in Makefile
SYSTEMC_HOME = /path/to/your/systemc
NIC_SIMULATOR_HOME = /path/to/your/network-simulator
TEST_NAME = packet_generator
```

### Compile and Run
```bash
# Compile
make all

# Run tests
make run

# Clean build files
make clean
```