# Pareas

GPU-accelerated compiler for a simple programming language, which outputs RISC-V machine code.

## Usage

The project consists of several binaries, some of which serve as tools during the compilation process:
* `pareas`, the compiler itself.
* `pareas-json`, a json parser implemented using similar techniques as the compiler.
* `pareas-lpg`, a lexer and parser generator for parallel lexers and parsers.

### The compiler

The basic usage of the Pareas compiler is
```
$ pareas <input path> -o <output path>
```
See `pareas --help` for additional options.

### The json parser

Usage of the json parser is similar to the compiler itself. There is no output, however. It simply parses the supplied json file and optionally prints some statistics.

### The lexer and parser generator

The lexer and parser generator is used to generate Futhark sources from a grammar definition, and its most basic invocation is
```
pareas-lpg --lexer <lexer grammar path> --parser <parser grammar path> -o <output basename> --namespace <namespace>

```

The output of this tools is a bunch of tables that are to be embedded in C++ and Futhark programs. The following files are generated:
* `<output basename>.hpp` contains C++ definitions of:
    * Token- and production constants.
    * Utility functions.
    * Structures.
    * Extern variables representing the generated tables.
* `<output basename>.cpp` contains the implementation of the utility functions, the definitions of the extern variables representing the generated tables.
* `<output basename>.dat` containing the actual generated tables.
* `<output basename>.S` containing an incbin statement for the generated data files.
* `<output basename>.fut` containing Futhark definitions for tokens and productions.

See `src/json/json.lex` and `src/json/json.g` for an example of how lexer and parser grammar files should look like.

## Project Structure

Pareas is built using the help of several tools which are also located in this project and are built as part of the compilation process. The project is laid out as follows:
* `src/tools/compile_futhark.py` is a tool used during building that helps with compiling Futhark. Normally, the Futhark compiler is invoked on a single source root and finds other imports by relative paths. This projects generates some Futhark files during it's build process. To avoid polluting the source directory, we copy the source tree of Futhark files into the source directory, where the generated files are also placed in. Generated files appear under the `gen` folder as if relative to the project root, so to import a generated file from `src/compiler/frontent.fut` one has to import `../../gen/generated_file`.
* `src/compiler/` contains the compiler itself. The Futhark files in this directory implement the meat of the compiler, while the c++ files implement some driving logic such as reading the input and writing the output.
* `src/json/` contains an example json parser implemented using similar techniques used for the main compiler.
* `src/lpg/` contains the lexer- and parser generator.
* `src/profiler/` contains a very simple profiler used for measuring the performance of the compiler.

The frontend of the project consists of `src/compiler/frontend.fut` and all the files it references, as well as `src/json/` and `src/lpg/`. The backend consists of `src/compiler/backend.fut` and all the files it references.

## Building

Building Pareas requires the following dependencies:
* A C++20-capable compiler such as clang or gcc.
* The [Meson](https://mesonbuild.com/) build system.
* [Ninja](https://ninja-build.org) or [Samurai](https://github.com/michaelforney/samurai) to build.
* A [Futhark](https://github.com/diku-dk/futhark) compiler. The latest tested version is 20.6.
* Python, which is required for Meson as well as some build tools included in the project.

Some additional dependencies such as `{fmt}` are automatically downloaded by Meson, and so this requires an active internet connection.

Dependending on the selected Futhark backend, some more dependencies might be requires:
* The OpenCL backend requires OpenCL development files.
* The CUDA backend requires CUDA development files.
* The multithreaded backend requires pthread.

To compile the project, please run:
```
$ mkdir build
$ cd build
$ meson .. -Dfuthark-backend=[opencl|cuda|c|multicore]
$ ninja
```

## Citing

This repository contains the source code from the following Master Thesis projects:
* Robin Voetter, "Parallel Lexing, Parsing and Semantic Analysis on the GPU", 2021, https://theses.liacs.nl/2052
* Marcel Huijben, "Parallel Code Generation on the GPU", 2021, https://theses.liacs.nl/2053

If you use any of the material provided in the repository, please cite the above.

```
@mastersthesis{voetter2021,
    author = {Robin Voetter},
    title = {Parallel Lexing, Parsing and Semantic Analysis on the GPU},
    school = {Leiden University},
    year = {2021},
}
```
```
@mastersthesis{huijben2021,
    author = {Marcel Huijben},
    title = {Parallel Code Generation on the GPU},
    school = {Leiden University},
    year = {2021},
}
```

## Troubleshooting

### OpenCL on AMD GPUs under Linux

Futhark requires OpenCL 1.2. Unfortunately, Mesa only implements OpenCL up to version 1.1. In order to work around this, one can install the AMDGPU-PRO version of OpenCL. Note that this library does not require the entire AMDGPU-PRO stack, just the OpenCL implementation is sufficient, and this works alongside Mesa.

To install the OpenCL implementation from AMDGPU-PRO, first download the latest version of the driver from [AMD](https://www.amd.com/en/support). Then run:
```
# Extract the package
$ tar xf amdgpu-pro-<version>.tar.xz
$ cd amdgpu-pro-<version>
# If you run a debian-based distro, this deb file can be installed directly.
$ ar x opencl-orca-amdgpu-pro-icd<another version>_amd64.deb
$ tar xf data.tar.xz
# Put the driver somewhere on the system
$ mv opt/amdgpu-pro/lib/x86_64-linux-gnu/libamdocl-orca64.so /usr/lib/libamdocl-orca64.so
# Make sure that the OpenCL ICD loader can find it
$ echo libamdocl-orca64.so > /etc/OpenCL/vendors/amdocl-orca64.icd
```

After this, `clinfo` should print an additional platform with name `AMD Accelerated Parallel PRocessing`. When invoking Pareas, make sure that the right device is selected by passing `pareas -d DeviceName`. `DeviceName` can be found in the output of `clinfo`.
