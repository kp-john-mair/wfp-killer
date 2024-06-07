## Overview

Inspect and manipulate the Windows WFP firewall.

## Build instructions

```sh
mkdir build/
cd build/
cmake ..
cmake --build .
```

### Usage

From the build folder:
`./src/Debug/wfpk.exe`

```
A tool to inspect and manipulate the WFP firewall.

Usage: wfpk.exe <subcommand>

Subcommands:
  create     create a WFP filter
  delete     Delete WFP objects
  list       List WFP objects
  monitor    Monitor WFP events

See wfpk.exe <subcommand> -h for detailed help.
```
