# Para C Language

## About
This is a custom programming language, similar in syntax to the C. Example:
```
// Fibonachi program

N = ?;
a = b = 1;
c = 0;
if (N <= 2)
    print 1;
else {
    i = 3;
    while (i <= N) {
      c = a + b;
      a = b;
      b = c;
      i = i + 1;
    }
    print b;
}

```
? - scan function which returns the read value to the call location.  
All types are integer (so far).  
This program has two modes of operation: interpreter and compiler. By default, the interpreter mode is enabled. By specifying **-oper-mode=compiler**, you'll get generated LLVM IR, which can then be linked with the paraCL standard library (**lib/std_pcl_lib/pcllib.cpp**). Using clang, you can then produce an executable file.
## Requirements
[Nix](https://nixos.org/download/) must be installed. 
## How to build
```bash
git clone git@github.com:VictorBerbenets/ParaCL.git
cd ParaCL
nix develop .

cmake -S ./ -B build/
cmake --build build
```

## To Run the program do
```bash
./build/paracl [options] <input-file>

# OPTIONS:
#
# Generic Options:
#
#   --help                             - Display available options (--help-hidden for more)
#   --help-list                        - Display list of available options (--help-list-hidden for more)
#   --version                          - Display the version of this program
#
# ParaCL options:
# Options for controlling the running process.
#
#   --module-name=<paraCL module name> - Set the name for the paraCL module
#   -o <filename>                      - Specify output filename for llvm IR
#   --oper-mode=<value>                - Set the operating mode
#     =compiler                        -   Compiling paraCL code into llvm IR
#     =interpreter                     -   Interpreting paraCL code without compiling
#   --target-triple=<string>           - Set the platform target triple
```
## How to run tests:
```bash
lit tests/
```
