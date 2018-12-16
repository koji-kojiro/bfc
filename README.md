# bfc
# Overview
This is a trivial [brainf\*ck](https://en.wikipedia.org/wiki/Brainfuck) compiler based on [libgccjit](https://gcc.gnu.org/wiki/JIT).
The aim of this project is understanding how to use libgccjit to implement a programming language and its compiler.

# Build from source
Clone this repo and then run `make`.

# Usage

```
usage: bfc [options] file

options:
  -e <string> run <string> directly
  -h          show this screen and exit
  -m <size>   specify memory size in bytes (default 1024)
  -o <file>   place the output into <file> (default a.out)
  -O <level>  set optimization level (from 1 to 3, default 0)
  -r          execute file directly
  -v          show version info and exit

```

# Author
[TANI Kojiro](https://github.com/koji-kojiro) (kojiro0531@gmail.com)

# License
CL-REPL is distributed under [GPLv3](./LICENSE).
