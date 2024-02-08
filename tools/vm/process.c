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

#define RVR_RW_IMPLEMENTATION
#define RVR_MALLOC_IMPLEMENTATION
#include "RvR_log.h"
#include "RvR_rw.h"
#include "RvR_malloc.h"
//-------------------------------------

//Internal includes
//-------------------------------------

//#defines
#define READ_ARG(I) \
        ((++(I))<argc?argv[(I)]:NULL)

#define MIN(a, b) ((a)<(b)?(a):(b))
//-------------------------------------

//Typedefs
typedef struct
{
   uint32_t type;
   uint32_t p_offset;
   uint32_t p_vaddr;
   uint32_t p_filesz;
   uint32_t p_memsz;
   uint32_t flags;
}Entry;
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static void print_help(char **argv);

static void process_ops(uint32_t *mem, int count);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   const char *path_in = NULL;
   const char *path_out = NULL;

   for(int i = 1; i<argc; i++)
   {
      if(strcmp(argv[i], "--help")==0||
         strcmp(argv[i], "-help")==0||
         strcmp(argv[i], "-h")==0||
         strcmp(argv[i], "?")==0)
      { print_help(argv); return 0; }
      else if(strcmp(argv[i], "-fin")==0)
         path_in = READ_ARG(i);
      else if(strcmp(argv[i], "-fout")==0)
         path_out = READ_ARG(i);
   }

   if(path_in==NULL)
   {
      RvR_log("input texture not specified, try %s -h for more info\n", argv[0]);
      return 0;
   }
   if(path_out==NULL)
   {
      RvR_log("output texture not specified, try %s -h for more info\n", argv[0]);
      return 0;
   }

   RvR_rw rw;
   RvR_rw_init_path(&rw, path_in, "rb");
   RvR_rw rw_out;
   RvR_rw_init_path(&rw_out, path_out, "wb");

   //header
   //Magic num
   RvR_error_check(RvR_rw_read_u8(&rw)==0x7f, "ELF header", "magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='E', "ELF header", "magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='L', "ELF header", "magic num incorrect");
   RvR_error_check(RvR_rw_read_u8(&rw)=='F', "ELF header", "magic num incorrect");

   //32/64bit
   RvR_error_check(RvR_rw_read_u8(&rw)==1, "ELF header", "incompatible architecture, expected 32bit");

   //Endianes
   RvR_error_check(RvR_rw_read_u8(&rw)==1, "ELF header", "incompatible endianess, expected little endian");

   //Skip version, OS ABI and 8 padding bytes
   for(int i = 0; i<10; i++)
      RvR_rw_read_u8(&rw);

   //Skip relocatetable/executable/shared/core field
   RvR_rw_read_u16(&rw);

   //Instruction set
   RvR_error_check(RvR_rw_read_u16(&rw)==0xf3, "ELF header", "incompatible instruction set, expected risc-v");

   //Skip elf version
   RvR_rw_read_u32(&rw);

   //Read actually needed info
   uint32_t entry_pos = RvR_rw_read_u32(&rw); //Program entry position
   uint32_t program_table_pos = RvR_rw_read_u32(&rw);
   uint32_t section_table_pos = RvR_rw_read_u32(&rw);
   uint32_t program_flags = RvR_rw_read_u32(&rw);
   uint16_t header_size = RvR_rw_read_u16(&rw);
   uint16_t program_entry_size = RvR_rw_read_u16(&rw);
   uint16_t program_entry_count = RvR_rw_read_u16(&rw);
   uint16_t section_entry_size = RvR_rw_read_u16(&rw);
   uint16_t section_entry_count = RvR_rw_read_u16(&rw);
   uint16_t section_table_names = RvR_rw_read_u16(&rw);
   RvR_log("|e_shnum |%16d|\n", section_entry_count);
   RvR_log("|-------------------------|\n");
   //-------------------------------------

   //program header
   Entry *entries = RvR_malloc(sizeof(*entries) * program_entry_count);
   uint32_t mem_size = 0;
   uint32_t entry_count = 0;
   RvR_rw_seek(&rw, program_table_pos, SEEK_SET);
   for(int i = 0; i<program_entry_count; i++)
   {
      entries[i].type = RvR_rw_read_u32(&rw);
      entries[i].p_offset = RvR_rw_read_u32(&rw);
      entries[i].p_vaddr = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      entries[i].p_filesz = RvR_rw_read_u32(&rw);
      entries[i].p_memsz = RvR_rw_read_u32(&rw);
      entries[i].flags = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);

      RvR_log("|type    |%16d|\n", entries[i].type);
      RvR_log("|p_offset|%16d|\n", entries[i].p_offset);
      RvR_log("|p_vaddr |%16d|\n", entries[i].p_vaddr);
      RvR_log("|p_filesz|%16d|\n", entries[i].p_filesz);
      RvR_log("|p_memsz |%16d|\n", entries[i].p_memsz);
      RvR_log("|p_flags |%16d|\n", entries[i].flags);
      RvR_log("|-------------------------|\n");

      if(entries[i].type!=1)
         continue;

      entry_count++;
      if(entries[i].p_vaddr + entries[i].p_memsz>mem_size)
         mem_size = entries[i].p_vaddr + entries[i].p_memsz;
   }
   //-------------------------------------

   //section header
   RvR_rw_seek(&rw, section_table_pos, SEEK_SET);
   uint32_t string_pos = 0;
   uint32_t code_size = 0;
   //Find string pos
   for(int i = 0; i<section_entry_count; i++)
   {
      RvR_rw_read_u32(&rw);
      uint32_t type = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      uint32_t offset = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      if(type==0x3)
         string_pos = offset;
   }
   RvR_rw_seek(&rw, section_table_pos, SEEK_SET);
   for(int i = 0; i<section_entry_count; i++)
   {
      uint32_t name_off = RvR_rw_read_u32(&rw);
      uint32_t type = RvR_rw_read_u32(&rw);
      uint32_t flags = RvR_rw_read_u32(&rw);
      uint32_t addr = RvR_rw_read_u32(&rw);
      uint32_t offset = RvR_rw_read_u32(&rw);
      uint32_t size = RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      RvR_rw_read_u32(&rw);
      //RvR_rw_read_u8(&rw);
      int pos = RvR_rw_tell(&rw);
      RvR_rw_seek(&rw, string_pos + name_off, SEEK_SET);
      char name[512];
      char ch;
      int i;
      for(i = 0; (ch = RvR_rw_read_u8(&rw))!='\0'; i++)
         name[i] = ch;
      name[i] = '\0';
      RvR_rw_seek(&rw, pos, SEEK_SET);
      RvR_log("|name    |%16s|\n", name);
      RvR_log("|addr    |%16d|\n", addr);
      RvR_log("|offset  |%16d|\n", offset);
      RvR_log("|size    |%16d|\n", size);
      if(strcmp(name, ".text")==0)
         code_size = size;
      RvR_log("|-------------------------|\n");
   }
   //-------------------------------------

   //Write new header
   RvR_rw_write_u32(&rw_out, entry_pos);
   RvR_rw_write_u32(&rw_out, mem_size);
   RvR_rw_write_u32(&rw_out, entry_count);
   uint32_t offset = 12 + entry_count * 16;
   for(int i = 0; i<program_entry_count; i++)
   {
      if(entries[i].type!=1)
         continue;

      RvR_rw_write_u32(&rw_out, offset);
      RvR_rw_write_u32(&rw_out, entries[i].p_vaddr);
      RvR_rw_write_u32(&rw_out, entries[i].p_filesz);
      RvR_rw_write_u32(&rw_out, entries[i].p_memsz);

      offset += entries[i].p_filesz;
   }
   //-------------------------------------

   //Write data sections
   for(int i = 0; i<program_entry_count; i++)
   {
      if(entries[i].type!=1)
         continue;

      void *mem = RvR_malloc(entries[i].p_filesz);
      RvR_rw_seek(&rw, entries[i].p_offset, SEEK_SET);
      RvR_rw_read(&rw, mem, 1, entries[i].p_filesz);
      if(entries[i].flags & 1)
         process_ops(mem, code_size / 4);
      RvR_rw_write(&rw_out, mem, 1, entries[i].p_filesz);
      RvR_free(mem);
   }
   //-------------------------------------

RvR_err:
   RvR_rw_close(&rw);
   RvR_rw_close(&rw_out);

   return 0;
}

static void print_help(char **argv)
{
   RvR_log("%s usage:\n"
           "%s -fin filename -fout filename\n"
           "   -fin        input texture path\n"
           "   -fout       output texture path\n",
           argv[0], argv[0]);
}

static void process_ops(uint32_t *mem, int count)
{
   for(int i = 0; i<count; i++)
   {
      uint32_t op = mem[i];
      uint32_t op_new = 0;

      //printf("0x%8x\n",op);

      int32_t arg0;
      int32_t arg1;
      int32_t arg2;
      int32_t arg3;
      int32_t arg4;
      switch(op & 127)
      {
      case 3: //OP_LOAD
         //I format
         arg3 = (op >> 7) & 31;
         arg2 = (op >> 12) & 7;
         arg1 = (op >> 15) & 31;
         arg0 = (op >> 20) & 4095;
         arg0 = (arg0 << 20) >> 20; //sign extend

         switch(arg2)
         {
         case 0:    //LB
            op_new = 0;
            break;
         case 1:    //LH
            op_new = 1;
            break;
         case 2:    //LW
            op_new = 2;
            break;
         case 4:    //LBU
            op_new = 3;
            break;
         case 5:    //LHU
            op_new = 4;
            break;
         }
         op_new |= arg1 << 6;
         op_new |= arg3 << 11;
         op_new |= arg0 << 16;

         break;
      case 19: //OP_IMM
         //I format
         arg3 = (op >> 7) & 31;
         arg2 = (op >> 12) & 7;
         arg1 = (op >> 15) & 31;
         arg0 = (op >> 20) & 4095;
         arg0 = (arg0 << 20) >> 20; //sign extend

         switch(arg2)
         {
         case 0:    //ADDI
            op_new = 5;
            break;
         case 1:    //SLLI
            op_new = 6;
            break;
         case 2:    //SLTI
            op_new = 7;
            break;
         case 3:    //SLTIU
            op_new = 8;
            break;
         case 4:    //XORI
            op_new = 9;
            break;
         case 5:     //SRLI/SRAI
            if(arg0 & 1024)
               op_new = 11;
            else
               op_new = 10;
            break;
         case 6:    //ORI
            op_new = 12;
            break;
         case 7:    //ANDI
            op_new = 13;
            break;
         }
         op_new |= arg1 << 6;
         op_new |= arg3 << 11;
         op_new |= arg0 << 16;

         break;
      case 35: //OP_STORE
         //S format --> saved as I format
         arg0 = (op >> 20) & 4064;
         arg1 = (op >> 20) & 31;
         arg2 = (op >> 15) & 31;
         arg3 = (op >> 12) & 7;
         arg0 |= (op >> 7) & 31;
         arg0 = (arg0 << 20) >> 20;

         switch(arg3)
         {
         case 0:    //SB
            op_new = 14;
            break;
         case 1:    //SH
            op_new = 15;
            break;
         case 2:    //SW
            op_new = 16;
            break;
         }
         op_new |= arg2 << 6;
         op_new |= arg1 << 11;
         op_new |= arg0 << 16;

         break;
      case 99: //OP_BRANCH
         //B format --> saved as I format
         arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op >> 19) & 4096) | ((op >> 20) & 2016)) | ((op >> 7) & 30)) | ((op << 4) & 2048)) << 19)) >> 19);
         arg1 = (op >> 20) & 31;
         arg2 = (op >> 15) & 31;
         arg3 = (op >> 12) & 7;

         switch(arg3)
         {
         case 0:    //BEQ
            op_new = 17;
            break;
         case 1:    //BNE
            op_new = 18;
            break;
         case 4:    //BLT
            op_new = 19;
            break;
         case 5:    //BGE
            op_new = 20;
            break;
         case 6:    //BLTU
            op_new = 21;
            break;
         case 7:    //BGEU
            op_new = 22;
            break;
         }
         op_new |= arg2 << 6;
         op_new |= arg1 << 11;
         op_new |= arg0 << 16;

         break;
      case 103: //OP_JALR
         //I format
         arg3 = (op >> 7) & 31;
         arg1 = (op >> 15) & 31;
         arg0 = (op >> 20) & 4095;
         arg0 = (arg0 << 20) >> 20; //sign extend

         op_new = 23;
         op_new |= arg1 << 6;
         op_new |= arg3 << 11;
         op_new |= arg0 << 16;

         break;
      case 115: //OP_SYSTEM
         //I format
         arg0 = (op >> 20) & 4095;
         arg0 = (arg0 << 20) >> 20; //sign extend

         op_new = 24;
         op_new |= arg0 << 16;

         break;
      case 51: //OP
         //R format
         arg0 = (op >> 25) & 127;
         arg1 = (op >> 20) & 31;
         arg2 = (op >> 15) & 31;
         arg3 = (op >> 12) & 7;
         arg4 = (op >> 7) & 31;

         switch((arg0 << 3) | arg3)
         {
         case 0:    //ADD
            op_new = 25;
            break;
         case 256:    //SUB
            op_new = 26;
            break;
         case 1:    //SLL
            op_new = 27;
            break;
         case 2:    //SLT
            op_new = 28;
            break;
         case 3:    //SLTU
            op_new = 29;
            break;
         case 4:    //XOR
            op_new = 30;
            break;
         case 5:    //SRL
            op_new = 31;
            break;
         case 261:    //SRA
            op_new = 32;
            break;
         case 6:    //OR
            op_new = 33;
            break;
         case 7:    //AND
            op_new = 34;
            break;
         case 8:    //MUL
            op_new = 35;
            break;
         case 9:    //MULH
            op_new = 36;
            break;
         case 10:    //MULHSU
            op_new = 37;
            break;
         case 11:    //MULHU
            op_new = 38;
            break;
         case 12:    //DIV
            op_new = 39;
            break;
         case 13:    //DIVU
            op_new = 40;
            break;
         case 14:    //REM
            op_new = 41;
            break;
         case 15:    //REMU
            op_new = 42;
            break;
         }
         op_new |= arg2 << 6;
         op_new |= arg1 << 11;
         op_new |= arg4 << 16;

         break;
      case 23: //OP_AUIPC
         //U format
         arg0 = op & 4294963200;
         arg1 = (op >> 7) & 31;

         op_new = 43;
         op_new |= arg1 << 6;
         op_new |= arg0;

         break;
      case 55: //OP_LUI
         //U format
         arg0 = op & 4294963200;
         arg1 = (op >> 7) & 31;

         op_new = 44;
         op_new |= arg1 << 6;
         op_new |= arg0;

         break;
      case 111: //OP_JAL
         //J format
         arg0 = (int32_t)(((int32_t)((uint32_t)(int32_t)(((((op >> 11) & 1048576) | ((op >> 20) & 2046)) | ((op >> 9) & 2048)) | ((op << 0) & 1044480)) << 11)) >> 11);
         arg1 = (op >> 7) & 31;

         op_new = 45;
         op_new |= arg1 << 6;
         op_new |= arg0 << 11;

         break;
      default: printf("Invalid opcode %d in word %d\n", op & 127, i);
      }

      mem[i] = op_new;
   }
}
//-------------------------------------
