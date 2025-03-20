# Para C Language

## About
ParaCL is a custom educational programming language designed to introduce foundational programming concepts in a simplified, learner-friendly environment. Below, we outline the basic capabilities of ParaCL, including variables, arrays, loops, if-else conditional statements, and basic input/output operations (print and scan). All types are integer (so far).  
### Variables
You can create variables of the integer type:
```
  ...
  // declare and define integer variable
  IamIntVar = 10;
  ...
```
There are two keywords available for arrays: ***array*** and ***repeat***. The first one is called `preset` array and it concatenates everything that is passed to it, e.g:
```
  IamPresetArr = array(10, undef, repeat(10, 3), array(1, 2)); // array(10, undef, 10, 10, 10, 1, 2)
```
> NOTE: ***array*** must be able to deduce its size, as a result, other arrays with unknown length cannot be transferred there. 

***repeat*** arrays creates with the initializer and size specified. Another array can act as an initializer.  Such arrays are called `uniform` in ParaCL because they are initialized with a single value/array, e.g:
```
  UniformArr = repeat(-1, 5); // -1 -1 -1 -1 -1
  UniformArr2 = repeat(UniformArr, 2); // 2 x UniformArr: -1 -1 -1 -1 -1 -1 -1 -1 -1 -1

  PresetArr = array(1, 2, 3);
  Size = 2;
  UniformArr3 = (PresetArr, Size); // 1 2 3 1 2 3
  ...
```
You can access the array through the usual square brackets syntax in C-like languages:  
```
 Arr = repeat(repeat(42, 3), 2); // 42 42 42 42 42 42
 Arr[0][1] = ...;
 print Arr[1][2];
 Arr[2][0] = ...; // error: out of range
```
### Control flow statements  
#### if else-if else
The syntax familiar from the C language is used to work with the conditions:
```
  ScanVal = ?;
  if (ScanVal > 0)
      print 9;
  else if (!ScanVal || ScanVal == -1)
      print 11;
  else {
      print 12;
      ScanVal = 5;
  }
```
#### while
The syntax for while is similar to the C language:
```
  Arr = array(-3, -2, undef, 0, 1, 2, undef);
  ArrSize = 7;
  Id = 0;
  while (Id < ArrSize) {
    print Arr[Id];
    Id = Id + 1; // there is no ++ operator
    ...
  }
```
### Functions
#### print
***print*** is a function of the standard library ParaCL. Accepts an int variable and outputs its value to cout:
```
  PrintMePlease = -11111111
  print PrintMePlease;
```
#### scan
***scan*** is a function of the standard library ParaCL. Returns the read integer value to the call point:
```
  ScanVal = ? + 5;
  print (ScanVal + ?) * ?;
  if (?)
    print ?;
```
## Requirements
[Nix](https://nixos.org/download/) must be installed. 
## How to build
```bash
git clone git@github.com:VictorBerbenets/ParaCL.git
cd ParaCL
nix --extra-experimental-features "nix-command flakes" develop .

cmake -S ./ -B build/
cmake --build build
```
After the project is successfully built, you have two modes to run the program.
1) Interpreted Mode (for Python lovers).  
Before execution, the code undergoes basic diagnostics to identify potential errors and only proceeds if none are found. This mode is enabled by default (can be explicitly activated via `-oper-mode=interpreter`).  
2) Compiler Mode.  
ParaCL code also undergoes error checks and then begins generating LLVM IR. By default, if no output file is specified (via -o), the result is written to the standart output. To enable this mode, submit `-oper-mode=compiler`.  
If you want to compile LLVM IR immediately and execute it, use the following command:  
```bash
  bash compiler.sh <input-file> [clang-options]
```
You will receive an executable file compiled and linked to the ParaCL standard library using clang.  
If you have an LLVM IR file and you want to build an executable file, submit it to clang for input by linking it to the ParaCL standard library - `lib/std_pcl_lib/pcllib.cpp`. E.g:  
```bash
  ./build/paracl -oper-mode=compiler main.pcl -o log.ll
  clang++ log.ll lib/std_pcl_lib/pcllib.cpp -o out
  ./out
```
## General view of the launch line
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
#   --dump-cfg=<dot file name>         - dump control flow graph in a dot file
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
## Example of the generated code:
### ParaCL code:  
```
ArrSize = ?;
Arr = repeat(10, ArrSize);

i = 0;
while (i < ArrSize) {
  print Arr[i];
  i = i + 1;
}
```
### dump-cfg option (useful for debug):  
![output](https://github.com/user-attachments/assets/2a4a7097-2bc3-41bd-814e-df9bb929d959)

