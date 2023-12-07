# Para C Language

## About
It is a simple programming language, similar in syntax to the C. The following operations are supported at early levels:  
while()  
if()  
print  
? - an analog of scanf()  
All types are integer (so far).
## Requirements
1. **cmake** version must be 3.15 or higher
2. [Bison & flex](https://www.gnu.org/software/bison/) must be installed
3. Gtest must be installed
## How to build
```bash
git clone --recurse-submodules https://github.com/VictorBerbenets/ParaCL.git
cd ParaCL
cmake -S ./ -B build/
cd build/
cmake --build .
```

## To Run the program do
```bash
./driver [file]
```
The program will be waiting for input file as command line argument with para C
code.
## How to run tests:
```bash
./example
```
