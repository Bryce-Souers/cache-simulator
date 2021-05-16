# Cache Simulator
Configurable 32-bit L1 cache simulator and benchmarking program.
This simulator takes in a trace file and reads what memory locations it accesses and how many bytes each access needs.

### Getting Started
`$ make` - Compiles the program to sim.o

`$ make run` - Compiles and runs 3 different configurations

`$ make run[1-8]` - Compile and run configurations 1 to 8

### Usage
| Flag  | Definition  |
| :------------ | :------------ |
| `-f`  | Input trace file  |
| `-s`  |  Cache size in KB (1KB to 8MB)  |
| `-b`  | Block size (4 to 64 bytes)  |
| `-a`  | Associativity (1, 2, 4, 8, 16)  |
|  `-r` | Replacement policy (RR, RND)  |

### Example Usage
###### Execution
`./sim.o -f inputs/trace1.trc -s 1024 -b 16 -a 8 -r RR`
###### Output
    Cache Simulator
    
    Trace File: inputs/trace1.trc
    
    ***** Cache Input Parameters *****
    Cache Size:                     1024 KB
    Block Size:                     16 bytes
    Associativity:                  8
    Replacement Policy:             Round Robin
    
    ***** Cache Calculated Values *****
    
    Total # Blocks:                 65536
    Tag Size:                       15 bits
    Index Size:                     13 bits
    Total # Rows:                   8192
    Overhead Size:                  131072 bytes
    Implementation Memory Size:     1152.00 KB (1179648 bytes)
    Cost:                           $103.68
    
    ***** CACHE SIMULATION RESULTS *****
    
    Total Cache Accesses:   387850 (350466 addresses)
    Cache Hits:             375156
    Cache Misses:           12694
    --- Compulsory Misses:     12677
    --- Conflict Misses:       17
    
    
    ***** ***** CACHE HIT & MISS RATE: ***** *****
    
    Hit  Rate:             96.7271%
    Miss Rate:             3.2729%
    CPI:                   4.57 Cycles/Instruction  (260322)
    Unused Cache Space:    929.16 KB / 1152.00 KB = 80.66%  Waste: $83.62
    Unused Cache Blocks:   52859 / 65536
