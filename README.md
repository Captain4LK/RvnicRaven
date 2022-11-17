# RvnicRaven

DIY game engine/framework, split into single headers, grab what you need, all licensed as CC0.

|library|description|latest version|dependencies|
|---|---|---|---|
|[RvR_log](headers/RvR_log.h)|adds logging to other headers if included before|1.0|None|
|[RvR_rw](headers/RvR_rw.h)|Data stream abstraction|1.0|None|
|[RvR_rand](headers/RvR_rand.h)|Collection of rngs, merely a abstraction over [rnd.h](https://github.com/mattiasgustavsson/libs)|1.0|None|
|[RvR_hash](headers/RvR_hash.h)|FNV hash implementation|1.0|None|
|[RvR_malloc](headers/RvR_malloc.h)|malloc replacement|1.0|None|
|[RvR_math](headers/RvR_math.h)|general purpose math|1.0|None|
|[RvR_fix22](headers/RvR_fix22.h)|22.10 fixed point math|1.0|None|
|[RvR_pak](headers/RvR_pak.h)|malloc replacement|1.0|RvR_hash, RvR_rw, RvR_malloc|
|[RvR_compress](headers/RvR_compress.h)|data compression|1.0|RvR_rw, RvR_malloc|
|[RvR_palette](headers/RvR_palette.h)|palette and color tables|1.0|RvR_rw, RvR_malloc, RvR_pak|
|[RvR_texture](headers/RvR_texture.h)|texture managment|1.0|RvR_rw, RvR_malloc, RvR_pak|
|[RvR_vm](headers/RvR_vm.h)|riscv bytecode vm (modding?)|1.0|RvR_rw, RvR_malloc|
|[RvR_ppp](headers/RvR_ppp.h)|predictor compression|1.0|RvR_rw, RvR_malloc|
|[RvR_core](headers/RvR_core.h)|framebuffer, input|1.0|RvR_palette, RvR_malloc|
|[RvR_draw](headers/RvR_draw.h)|basic drawing routines|1.0|RvR_core|
|[RvR_ray](headers/RvR_ray.h)|advanced raycasting|1.0|RvR_core,RvR_math,RvR_fix22 TODO|

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
