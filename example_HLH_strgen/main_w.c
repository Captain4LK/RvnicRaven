/*
HLH_strgen_w example

Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>
//-------------------------------------

//Internal includes
#define HLH_ERROR_IMPLEMENTATION
#include "../HLH_error.h"
#define HLH_STRGEN_IMPLEMENTATION
#include "../HLH_strgen_w.h"
//-------------------------------------

//#defines

//Universal dynamic array
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int main()
{
   srand(time(NULL));

   //Set locale to make sure utf8
   //is actually supported
   setlocale(LC_ALL,"en_US.utf8");

   //Create a new, empty second order markov chain
   HLH_strgen *str = HLH_strgen_new(2);
   if(str==NULL)
      puts(HLH_error_get_string());

   //Open a file and add its contents to the markov chain
   //The file used in this case is a list of some roman emperors
   //Latin names work especially well with markov chains
   FILE *f = fopen("emperors.txt","rb");
   HLH_strgen_add_file(str,f);
   if(f!=NULL)
      fclose(f);

   //Save the generated markov chain to disk
   //This can be skipped since it only is here for validation 
   //purposes
   f = fopen("test.bin","wb");
   if(HLH_strgen_model_save(str,f))
      puts(HLH_error_get_string());
   if(HLH_strgen_destroy(str))
      puts(HLH_error_get_string());
   if(f!=NULL)
      fclose(f);

   //Load the markov chain from disk again
   //This can be skipped since it only is here for validation 
   //purposes
   f = fopen("test.bin","rb");
   str = HLH_strgen_model_load(f);
   if(f!=NULL)
      fclose(f);

   //Generate 10 sentences
   //HLH_strgen does not do any filtering of
   //generated strings, this means
   //that generated strings can be the same as
   //the input strings
   for(int i = 0;i<10;i++)
   {
      wchar_t *s = HLH_strgen_generate(str);
      if(s!=NULL)
      {
         wprintf(L"%ls\n",s);
         free(s);
      }
   }

   //Free the markov chain 
   if(HLH_strgen_destroy(str))
      puts(HLH_error_get_string());

   return 0;
}
//-------------------------------------
