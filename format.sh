#!/bin/sh

uncrustify -c uncrustify.cfg --no-backup --replace $(find headers/ -name "*.c") $(find headers/ -name "*.h")
