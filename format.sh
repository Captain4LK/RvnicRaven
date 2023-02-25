#!/bin/sh

src=$(find RvR/ -name "*.c")
src=$src $(find RvR/ -name "*.h")
src="$src $(find include/ -name "*.h")"
echo "$src"

uncrustify -c uncrustify.cfg --no-backup --replace $src
