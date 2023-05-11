# AccessString
A commandline tool for finding strings in binary files and the assembler
code that access it.
![output](https://github.com/danielfvm/AccessString/assets/23420640/7ae0bde3-5a30-4d0e-b2ec-2f9a69fa3704)

# Usage
```
Usage:
  accessstring <file> <string> [max_offset]

  file       Path to executable file
  string     String to be referenced
  max_offset Allow provided string to be offset by value, default 0
```

# Installation
You will need the [udis869](https://github.com/vmt/udis86) library to compile and install this program.

## Linux
The following commands will download, compile and install this program. Keep in mind that you porbably will
need root permission for running `make install`, which will then copy the binary file to `/usr/bin/`
```
git clone https://github.com/danielfvm/AccessString
cd AccessString
make
make install
```
