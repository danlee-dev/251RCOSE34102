# CPU Scheduling Simulator

A comprehensive CPU scheduling simulator that implements and compares various scheduling algorithms with multi-I/O support and real-time system analysis.

## Features

- **9 scheduling algorithms** implementation and performance comparison
- **Multi-I/O operations** support (up to 3 I/O operations per process)
- **Real-time system scheduling** (EDF, RMS) with theoretical analysis
- **Gantt chart visualization** and detailed performance metrics
- **Process configuration save/load** functionality
- **Comprehensive analysis reports** with recommendations

## Project Structure

```
CPU_Scheduling_Simulator/
├── src/                    # Source code files
│   ├── main.c             # Program entry point
│   ├── scheduler.c        # Scheduling algorithms implementation
│   ├── process.c          # Process management and generation
│   ├── evaluation.c       # Performance evaluation and output
│   ├── queue.c            # Queue implementation for scheduling
│   ├── config.c           # System configuration management
│   └── sort_utils.c       # Sorting utilities
├── include/               # Header files
│   ├── scheduler.h
│   ├── process.h
│   ├── evaluation.h
│   ├── queue.h
│   ├── config.h
│   └── sort_utils.h
├── test_files/            # Process configuration files
│   ├── process-1.txt
│   ├── process-2.txt
│   └── ...
├── result_example/        # Analysis reports storage
├── obj/                  # Compiled object files
├── Makefile              # Build configuration
└── README.md
```

## Implemented Scheduling Algorithms

### General Scheduling Algorithms
- **FCFS (First-Come, First-Served)**: Executes processes in arrival order
- **SJF (Shortest Job First)**
  - Non-preemptive: Shortest job executes first
  - Preemptive: Preempts when shorter job arrives
- **Priority Scheduling**
  - Non-preemptive: Executes based on priority
  - Preemptive: Preempts when higher priority arrives
- **Round Robin**: Time quantum-based circular execution
- **Priority with Aging**: Priority scheduling with starvation prevention

### Real-time Scheduling Algorithms
- **EDF (Earliest Deadline First)**: Dynamic priority based on deadlines
- **RMS (Rate Monotonic Scheduling)**: Static priority based on periods

## Build and Execution

### Compilation
```bash
make
```

### Execution
```bash
./cpu_simulator
```

### Clean
```bash
make clean
```

## Usage

### 1. Process Generation Mode Selection
```
random create mode? ('y': yes, 'n': no, 'f': Use file):
```
- **'y'**: Generate random processes
- **'n'**: Manual process input
- **'f'**: Load processes from file

### 2. File Mode Usage
- Available files in `test_files/` directory are displayed
- Select the desired file number to load processes

### 3. Scheduling Algorithm Selection
```
===== CPU Scheduling Algorithms =====
1. Run FCFS
2. Run SJF (Non-preemptive)
3. Run SJF (Preemptive)
4. Run Priority (Non-preemptive)
5. Run Priority (Preemptive)
6. Run Round Robin
7. Run Priority with Aging (Preemptive)
8. Run RMS (Rate Monotonic Scheduling)
9. Run EDF (Earliest Deadline First)
10. Compare all algorithms
11. Exit
```

## Output Results

### 1. Gantt Chart
Visual representation of process execution timeline

### 2. Process Information Table
- Basic process information (PID, burst time, arrival time, priority, etc.)
- Multi-I/O operation details
- Completion time, waiting time, turnaround time

### 3. Performance Metrics
- Average Waiting Time
- Average Turnaround Time
- CPU Utilization
- Throughput
- I/O statistics and analysis

### 4. Real-time System Analysis (EDF, RMS)
- System utilization analysis
- Schedulability prediction
- Deadline miss logging
- Theoretical bound comparison

### 5. Comprehensive Analysis Report
- Performance comparison between algorithms
- Detailed analysis and recommendations
- System characteristics analysis
- Efficiency score calculation

## File Format

### Process Configuration File (test_files/)
```
4                           # Number of processes
0 4 10 6 28 50 1            # PID ArrivalTime CPUBurst Priority Deadline Period IOCount
6 3                         # IO StartTime IOBurst
1 8 8 4 35 60 2             # Process 1 information
1 5                         # IO 1
5 2                         # IO 2
...
```

## Key Features

### Multi-I/O Support
- Each process can perform up to 3 I/O operations
- Processes enter waiting state during I/O operations
- Return to ready queue after I/O completion

### Real-time Scheduling Analysis
- **EDF**: Theoretically schedulable up to 100% CPU utilization
- **RMS**: Liu & Layland theoretical bound (≈75.7% for 4 processes)
- Deadline miss tracking and logging

### Priority Aging Mechanism
- Priority increases when waiting time exceeds threshold (3)
- Prevents starvation
- Tracks priority change history

### Performance Analysis Tools
- Simultaneous comparison of 9 algorithms
- Efficiency score calculation
- System characteristics analysis (CPU vs I/O intensive)
- Automatic comprehensive report generation

## System Requirements

- GCC compiler
- Make build tool
- Linux/Unix environment (for directory structure support)
- Math library (libm)

## Development and Testing Environment

This project was developed and tested on:
- **macOS Sequoia (Version 15.5)**
- **Ubuntu 24.04.2 LTS** running on UTM virtual machine

The simulator is designed to work seamlessly in both macOS and Linux environments, with primary testing conducted on Ubuntu LTS for compatibility assurance.

## Directory Setup

Ensure the following directories exist before running the program:

```bash
mkdir -p test_files result_example
```

## Usage Examples

### 1. File Mode Execution
```bash
./cpu_simulator
# Select 'f' → Choose file → Run algorithm
```

### 2. All Algorithms Comparison
```bash
# Select option 10 → Enter max_time → Enter report filename(saved as result_example/filename.txt)
```

### 3. Save Process Configuration
```bash
# Run random mode ('y') → Test algorithms → Save with 'y'(saved as test_files/process-filename.txt)
```

## Performance Analysis Interpretation

- **Lower waiting time**: Better for interactive systems
- **Lower turnaround time**: Better for batch processing
- **Higher CPU utilization**: Better resource efficiency
- **Fewer deadline misses**: Better for real-time systems

## Contributing

1. Fork the Project
2. Create your Feature Branch (`git checkout -b feature/AmazingFeature`)
3. Commit your Changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the Branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## License

This project is distributed under the MIT License. See [`LICENSE`](LICENSE) file for more information.
