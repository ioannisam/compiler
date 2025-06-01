# MIXAL Implemetation

This branch implements code generation using MIXAL instead of x86 assembly. For the complete implementation refer to the `main` branch.

## Tools (extra)

- **Docker**: For containerization and compatibility issues
- **mixasm**: Assembler for converting MIXAL code into object files
- **mixvm**:  Virtual Machine for running MIXAL executables.

## Apendix

In the famous series of books “The Art of Computer Programming,” published by D. Knuth, an idealized computer called MIX is used. MIX has a machine language and a symbolic (assembly) representation of its instructions. The architecture of MIX is a simplified version of real CISC CPU designs, and its symbolic language, called MIXAL, provides a set of basic instructions that will be very familiar to anyone with experience programming in assembly language. The definition of MIX and MIXAL is powerful and comprehensive enough to offer an ideal platform for developing complex programs—quite close to real computers—without the complexity of those architectures. For this reason, MIX and MIXAL are widely used as an educational environment for learning programming techniques. For more information about MIXAL and the MIX architecture, refer to Donald Knuth's "The Art of Computer Programming" series.