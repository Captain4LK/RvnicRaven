/*
   Markov chain string generator

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <time.h>
//-------------------------------------

//Internal includes

#define HLH_MARKOV_ORDER_CHAR 3
#define HLH_MARKOV_ORDER_MIN_CHAR 2
#define HLH_MARKOV_ORDER_WORD 3
#define HLH_MARKOV_ORDER_MIN_WORD 2
#define HLH_MARKOV_IMPLEMENTATION
#include "../HLH_markov.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
static char *file_read(const char *path);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   srand(time(NULL));

   HLH_markov_model *model_word = HLH_markov_model_new(HLH_MARKOV_WORD);

   char *file = file_read("test_word.txt");
   char *str = file;
   char *ptr = file;
   while(*ptr++!='\0')
   {
      if(*ptr=='\n')
      {
         *ptr = '\0';
         HLH_markov_model_add(model_word,str);
         ptr++;
         str = ptr;
      }
   }
   free(file);

   for(int i = 0;i<1;i++)
   {
      char *text = HLH_markov_model_generate(model_word);
      puts(text);
      free(text);
   }
   HLH_markov_model_delete(model_word);

   HLH_markov_model *model_char = HLH_markov_model_new(HLH_MARKOV_CHAR);

   file = file_read("test_char.txt");
   str = file;
   ptr = file;
   while(*ptr++!='\0')
   {
      if(*ptr=='\n')
      {
         *ptr = '\0';
         HLH_markov_model_add(model_char,str);
         ptr++;
         str = ptr;
      }
   }
   free(file);

   for(int i = 0;i<16;i++)
   {
      char *text = HLH_markov_model_generate(model_char);
      puts(text);
      free(text);
   }
   HLH_markov_model_delete(model_char);

   return 0;
}

static char *file_read(const char *path)
{
   FILE *f = fopen(path,"r");
   int size = 0;
   fseek(f,0,SEEK_END);
   size = ftell(f);
   fseek(f,0,SEEK_SET);
   char *text = malloc(sizeof(*text)*(size+1));
   fread(text,size,1,f);
   text[size] = '\0';
   fclose(f);

   return text;
}
//-------------------------------------
