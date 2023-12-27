gcc -o ported *.c -I../../external -I../../include ../../RvR_portal/libRvR_port.a ../../RvR/libRvR.a -lSDL2 -g -O3 -Wall -Wextra -Wno-sign-compare -lprofiler -fsanitize=undefined,address
gdb --args ./ported  $1
