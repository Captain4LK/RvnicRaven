/*
RvnicRaven retro game engine

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <inttypes.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#define RVR_RW_IMPLEMENTATION
#define RVR_MALLOC_IMPLEMENTATION
#define RVR_VM_IMPLEMENTATION
#include "RvR_log.h"
#include "RvR_rw.h"
#include "RvR_malloc.h"
#include "RvR_vm.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[1<<25];
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   RvR_malloc_init(mem,1<<25);

   RvR_vm vm = {0};
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw,"test.bin","rb");
   RvR_vm_create(&vm,&rw);
   RvR_rw_close(&rw);

   //RvR_vm_disassemble(&vm);
   RvR_vm_run(&vm,vm.pc_entry);

   return 0;
}
//-------------------------------------
