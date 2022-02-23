# HLH_headers

single-file public domain (CC0) libraries for C

|library|description|latest version|
|---|---|---|
|[HLH_markov](HLH_markov.h)|Markov chain-based procedural string (names, sentences) generation|1.0|
|[HLH_math](HLH_math.h)|floating point math, vector and matrix math|wip|

# How to use

Every header contains basic instructions on how to include in your project as well as a list of preprocessor options available. 

Basically all you need to do is include the header like you would any other. Additionally you need to add a preprocessor directive in ONE C file before including the header to create the implementation:

```C
//Do this ONCE in a .c file
#define LIBNAME_IMPLEMENTATION
#include "libname.h"

//In all other C files, just include the header
#include "libname.h"
```

# License

All code in this repository is released into the public domain (CC0), see COPYING for more info.
