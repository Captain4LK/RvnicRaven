#!/bin/sh

src=$(find headers/ -name "*.c")
src="$src $(find headers/ -name "*.h")"
src="$src $(find tools/ -name "*.c")"
src="$src $(find tools/ -name "*.h")"
echo "$src"

uncrustify -c uncrustify.cfg --no-backup --replace $src
