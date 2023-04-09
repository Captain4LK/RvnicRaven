#!/bin/sh

src=$(find RvR/ -name "*.c")
src=$src $(find RvR/ -name "*.h")
src=$src $(find RvR_port/ -name "*.c")
src=$src $(find RvR_port/ -name "*.h")
src=$src $(find RvR_ray/ -name "*.c")
src=$src $(find RvR_ray/ -name "*.h")
src="$src $(find include/ -name "*.h")"
src=$src $(find tools/ -name "*.c")
src=$src $(find tools/ -name "*.h")
echo "$src"

uncrustify -c uncrustify.cfg --no-backup --replace $src
