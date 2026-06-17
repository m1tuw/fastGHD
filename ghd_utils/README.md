# Build Instructions for fastGHD ghd_utils

This directory contains utility programs for testing and brute-force tree decomposition.

## Prerequisites

You need:
- **gcc** (C++17 compatible, used as the C++ driver)
- Either **mingw32-make** or **PowerShell** for building

## Compilation Options

### Option 1: Using mingw32-make (Recommended)

```powershell
cd ghd_utils
mingw32-make all       # Build everything
mingw32-make tests     # Build only tests
mingw32-make clean     # Remove all executables
```

### Option 2: Using PowerShell Script

```powershell
cd ghd_utils
powershell -ExecutionPolicy Bypass -File .\build.ps1 all
powershell -ExecutionPolicy Bypass -File .\build.ps1 clean
powershell -ExecutionPolicy Bypass -File .\build.ps1 tests
```

## Available Targets

- **hypertree_test**: Tests for hypertree decomposition checking
- **fec_test**: Tests for fractional edge cover solver
- **tree_decomp_brute**: Brute-force tree decomposition tool
- **tests**: Builds both test executables
- **all**: Builds everything (default)
- **clean**: Removes all compiled executables

## Running Tests

```powershell
cd ghd_utils

# Run individual tests
.\hypertree_test.exe
.\fec_test.exe

# Run brute-force decomposition
.\tree_decomp_brute.exe
```

## Project Structure

```
fastGHD/
├── ghd_utils/
│   ├── Makefile           (updated with correct paths)
│   └── build.ps1          (PowerShell build script)
├── src/
│   ├── hypergraphs/
│   │   └── hypertree_check.cpp
│   ├── fractional_edge_cover/
│   │   └── fractional_edge_cover_solver.cpp
│   └── tree_decomp_brute/
│       └── tree_decomp_brute.cpp
├── test/
│   ├── hypergraphs/
│   │   └── hypertree_check_test.cpp
│   └── fractional_edge_cover/
│       └── fractional_edge_cover_test.cpp
└── includes/
    └── (header files)
```

## Troubleshooting

### "make: command not found"
Use the PowerShell script instead:
```powershell
powershell -ExecutionPolicy Bypass -File .\build.ps1 all
```

### "gcc: command not found"
Install MSYS2/MinGW and ensure `gcc.exe` is on your `PATH`.

### Compilation errors about missing headers
Ensure all header files are in `../includes/` directory.

### Warning: unused variable
These are non-critical warnings that don't prevent compilation.

