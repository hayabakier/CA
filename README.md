# McHarvard with Cheese — Arithmetic Shifts 🍔

A 5-stage pipelined CPU simulator built around a Harvard memory architecture (separate instruction and data memories), complete with hazard detection, forwarding, and a restaurant-themed Tkinter GUI.

## Overview

This project simulates a simple pipelined processor with a custom instruction set. It models the classic **Fetch → Decode → Execute → Memory → Writeback** pipeline, handling data hazards via forwarding and stalling, and control hazards via pipeline flushing (e.g. on `BEQZ` / `JR`).

The simulator can be run either from the terminal (raw cycle-by-cycle trace output) or through the included Python GUI for a more visual, stage-by-stage walkthrough.

## Instruction Set

| Opcode | Mnemonic | Description                     |
|-------:|----------|----------------------------------|
| 0      | `ADD`    | Add                              |
| 1      | `SUB`    | Subtract                         |
| 2      | `MUL`    | Multiply                         |
| 3      | `LDI`    | Load immediate                   |
| 4      | `BEQZ`   | Branch if equal to zero          |
| 5      | `AND`    | Bitwise AND                      |
| 6      | `OR`     | Bitwise OR                       |
| 7      | `JR`     | Jump register                    |
| 8      | `SAL`    | Shift arithmetic left            |
| 9      | `SAR`    | Shift arithmetic right           |
| 10     | `LB`     | Load byte (from data memory)     |
| 11     | `SB`     | Store byte (to data memory)      |

Register file: 64 registers (`R0`–`R63`). Status register flags: `C` (carry), `V` (overflow), `N` (negative), `S` (sign), `Z` (zero).

## Project Structure

```
McHarvard with Cheese Arithmetic Shifts/
├── main.c        # Simulation driver / pipeline loop
├── defs.h        # Shared types, constants, and function declarations
├── parser.c      # Assembly program parser and instruction encoder
├── FD.c          # Fetch and Decode stages
├── execute.c     # Execute stage
├── memory_WB.c   # Memory access and Writeback stages
├── hazard.c      # Hazard detection / forwarding / stalling logic
├── pipeline.c    # Pipeline latch handling
├── memory.c      # Instruction/data memory definitions
├── GUI.py        # Tkinter-based visual simulator front-end
├── program1.txt … program4.txt   # Sample assembly programs
└── How to run project from terminal.txt
```

## Building

Requires `gcc` (or any C compiler) and, for the GUI, Python 3 with Tkinter.

```bash
gcc main.c memory.c parser.c pipeline.c hazard.c memory_WB.c FD.c execute.c -o sim
```

## Running

### Terminal

The simulator reads its instructions from a file named `program.txt` in the working directory. Copy (or rename) one of the sample programs before running:

```bash
cp program1.txt program.txt
./sim        # or sim.exe on Windows
```

This prints a full cycle-by-cycle pipeline trace followed by the final register file, status flags, and non-zero memory contents.

### GUI

```bash
python GUI.py
```

The GUI lets you load a program, step or run the simulation, and watch instructions move through the IF / ID / EX / MEM / WB stages in real time.

## Sample Programs

Four sample assembly programs (`program1.txt`–`program4.txt`) are included to exercise arithmetic, logic, shift, memory, and branch/jump instructions.

## Documentation

- `Project Decription.pdf` — original project specification
- `Project Report.pdf` — write-up of the design and implementation
- `Project Demo.mp4` — video walkthrough/demo

## Contributors


- Toqa Wael
- Haya Bakier
- Shahd Walid
- Yassmin Ayman
- Layal Maged
- Hana Eissa

