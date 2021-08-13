# HLH_headers

single-file public domain (CC0) libraries for C

|library|description|latest version|
|---|---|---|
|[HLH_strgen](HLH_strgen.h)|Markov chain-based procedural string (names etc.) generation|1.0|
|[HLH_strgen_w](HLH_strgen_w.h)|wchar version of HLH_strgen|1.0|
|[HLH_error](HLH_error.h)|Error reporting extension, adds error codes and error strings to the other HLH libraries|1.0|

# Features

* **No dependencies**, uses only types/functions provided by the C99 standard, some of which can be overwritten using the preprocessor
* **Pure C99**, builds without warnings or errors using the ``-std=c99 -Wall -Wextra -pedantic`` flags on gcc
* **Single header**
* **Save**, every standard library function that can fail is checked and handled accordingly

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
