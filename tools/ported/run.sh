gcc -o ported *.c unix/*.c ../../external/tinyfiledialogs.c -I../../external -I../../include ../../RvR_portal/libRvR_port.a ../../RvR/libRvR_gui.a -lSDL2 -lm -g -O3 -Wall -Wextra -Wshadow -Wno-sign-compare -Wconversion -Wno-sign-conversion
gdb --args ./ported  $1
