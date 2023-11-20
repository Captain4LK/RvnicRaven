gcc -o ported *.c -I../../external -I../../include ../../RvR_portal/libRvR_port.a ../../RvR/libRvR.a -lSDL2 -g -Og -Wall -Wextra -fsanitize=undefined,address -Wno-sign-compare
./ported $1
