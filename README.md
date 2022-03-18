# Le Debug - LDB

## About

This project was done for the M1 CHPS **AISE** course at the University of Versailles-St-Quentin in France . This
project uses `cmake`, and the **build process** is detailed at the end of this document.

Launching the project :

```bash
$ ./ldb (<command> <arg1> <arg2> ...)
```

Where `<command>` is an **optional path** to an executable file that needs debugging. If no command is specified, the
debugger will start in a blank state and await a command.

The debugger displays a GUI where you can restart, pause, step, or kill the program. The terminal on the bottom right
shows the output of the tracee. The main panel shows off a dissasembled version of the program, and its source code (if
available). The bottom panel shows the stack, registers, and loaded libraries.

Breakpoints can be set on the source code using the red 'stop' button in the top toolbar.

## Design choice

The debugger is split into 4 parts :

* The GUI
* The ELF parser
    * Parses the elf file looking for local symbols (name and address). At a later stage, this parser will look for the
      loaded dynamic library of the executable, and recursively parse them, to provide additional symbols.
* The DWARF parser
    * Parses the dwarf file looking for additional debug information, such as variable names.
* The breakpoint system
    * Uses the functions address found using the previous parsers to set breakpoints. Since we're not able to determine
      which instruction corresponds to which line in the source file, we only offer breakpoints on the function level,
      and instruction-wise.

### Launching process :

1. Fork a child process, which launches the program to debug.
2. Parse static symbols
3. Set a breakpoints in '_start', and wait for the child to stop again
4. Parse dynamic symbols _(in _start, the shared libraries are loaded)_
5. Remove the breakpoint
6. Resume the program
7. Start an event loop

## Build

### Dependencies

The following packages name refer to one provided by **DNF**. Please refer to your distribution's package manager for
more information.

* Qt6
    * `qt6-qtbase-devel`
    * `qt6-qtcharts-devel`
* TBB
    * `tbb-devel`
* BOOST
    * `boost-devel`
* libdwarf*
    * `libdwarf-devel`
* libelf*
    * `libelf-devel`
* libunwind*
    * `libunwind-devel`

_*Note: We tested this project using **intel-oneapi** versions of those dependencies, which caused weird behaviour in
the parser, including segfaults in internal functions. The exact same code works fine with the official versions._

### Compilation

Typical build command:

```bash
sudo dnf install -y qt6-qtbase-devel qt6-qtcharts-devel tbb-devel \
  boost-devel libdwarf-devel libelf-devel libunwind-devel

git clone https://github.com/LDB-chps/LDB ldb && cd ldb
mkdir release-build && cd release-build

cmake .. -DCMAKE_BUILD_TYPE=Release
make -j ldb  
```



