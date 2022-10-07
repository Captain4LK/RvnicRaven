# RvnicRaven

DIY game engine/framework, split into single headers, grab what you need, all licensed as CC0.

|library|description|latest version|dependencies|
|---|---|---|---|
|[RvR_rw](headers/RvR_rw.h)|Data stream abstraction|1.0|None|
|[RvR_rand](headers/RvR_rand.h)|Collection of rngs, merely a abstraction over [rnd.h](https://github.com/mattiasgustavsson/libs)|1.0|None|

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
