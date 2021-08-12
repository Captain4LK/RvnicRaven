#ifndef _HLH_STRGEN_H_

/*
   Markov chain string generator

   Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define HLH_STRGEN_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

/*
   malloc(), realloc(), free() and rand() can be overwritten by 
   defining the following macros:

   HLH_STRGEN_MALLOC
   HLH_STRGEN_FREE
   HLH_STRGEN_REALLOC
   HLH_STRGEN_RAND
*/

#define _HLH_STRGEN_H_

typedef struct
{
   uint32_t used;
   uint32_t size;
   void *data;
}HLH_strgen_dyn_array;

typedef struct
{
   HLH_strgen_dyn_array chain;
   HLH_strgen_dyn_array chain_start;
   uint32_t order;
}HLH_strgen;

//Create a new, empty model
//Returns pointer to new model or NULL on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_ARG_OOR
//
//Parameters:
//    uint32_t order - how many characters to consider for choosing next character
//                     higher values result in strings being less random and more similar to input strings
//                     order >= 1
HLH_strgen *HLH_strgen_new(uint32_t order);

//Destroy a model
//Returns 0 on success or 1 on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_ARG_NULL
//
//Parameters:
//    HLH_strgen *str - the model to be freed
//                      str != NULL
int HLH_strgen_destroy(HLH_strgen *str);

//Parse a file and add every line to the model
//Returns 0 on success or 1 on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_FAIL_REALLOC
//    HLH_ERROR_ARG_NULL
//    HLH_ERROR_ARG_OOR
//
//Parameters:
//    HLH_strgen *str - model to add lines to
//                      str != NULL
//    FILE *f - file to read lines from
//              f != NULL
int HLH_strgen_add_file(HLH_strgen *str, FILE *f);

//Add a null terminated string to the model,
//Returns 0 on success or 1 on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_FAIL_REALLOC
//    HLH_ERROR_ARG_NULL
//    HLH_ERRROR_ARG_OOR
//
//Parameters:
//    HLH_strgen *str - model to add string to
//                      str != NULL
//    const char *s - string to add to model
//                    str != NULL
//                    strlen(s)+1 >= str->order
int HLH_strgen_add_string(HLH_strgen *str, const char *s);

//Generate a string, must be free()'d by user
//Returns pointer to generated string or NULL on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_FAIL_REALLOC
//    HLH_ERROR_ARG_NULL
//    HLH_ERROR_ARG_OOR
//
//Parameters:
//    HLH_strgen *str - the model to generate a string from 
//                      str != NULL
//                      str->chain_start.used != 0
char *HLH_strgen_generate(HLH_strgen *str);

//Write a model to disk
//Returns 0 on success or 1 on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_FWRITE
//    HLH_ERROR_ARG_NULL
//
//Parameters:
//    HLH_strgen *str - the model to be written to disk
//                      str != NULL
//    FILE *f - file to write model to
//              f != NULL
int HLH_strgen_model_save(const HLH_strgen *str, FILE *f);

//Read a model from disk
//Returns pointer to loaded model or NULL on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_FAIL_REALLOC
//    HLH_ERROR_FAIL_FWRITE
//    HLH_ERROR_FAIL_FSEEK
//    HLH_ERROR_FAIL_FTELL
//    HLH_ERROR_ARG_NULL
//    HLH_ERROR_ARG_OOR
//    HLH_ERROR_BUFFER_SHORT
//
//Parameters:
//    FILE *f - file to read model from
//                    f != NULL
HLH_strgen *HLH_strgen_model_load(FILE *f);

//Read a model from memory buffer
//Returns pointer to loaded model or NULL on failure
//Possible errors (only reported if HLH_error is used too):
//    HLH_ERROR_FAIL_MALLOC
//    HLH_ERROR_FAIL_REALLOC
//    HLH_ERROR_ARG_NULL
//    HLH_ERROR_ARG_OOR
//    HLH_ERROR_BUFFER_SHORT
//
//Parameters:
//    uint8_t *data - memory buffer to read model from
//                    data != NULL
//    unsigned length - size of buffer data points to
//                      length > 0
HLH_strgen *HLH_strgen_model_load_mem(const uint8_t *data, unsigned length);

#endif

#ifdef HLH_STRGEN_IMPLEMENTATION
#ifndef HLH_STRGEN_IMPLEMENTATION_ONCE
#define HLH_STRGEN_IMPLEMENTATION_ONCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifndef HLH_ERROR_FAIL
#define HLH_ERROR_FAIL(X) do { goto HLH_err; } while(0)
#endif

#ifndef HLH_ERROR_CHECK
#define HLH_ERROR_CHECK(X,Y) do { if(!(X)) HLH_ERROR_FAIL(Y); } while(0)
#endif

#ifndef HLH_STRGEN_MALLOC
#define HLH_STRGEN_MALLOC malloc
#endif

#ifndef HLH_STRGEN_FREE
#define HLH_STRGEN_FREE free
#endif

#ifndef HLH_STRGEN_REALLOC
#define HLH_STRGEN_REALLOC realloc
#endif

#ifndef HLH_STRGEN_RAND
#define HLH_STRGEN_RAND rand
#endif

#define HLH_dyn_array_init(type, array, space) \
   do\
   {\
      ((HLH_strgen_dyn_array *)(array))->size = (space);\
      ((HLH_strgen_dyn_array *)(array))->used = 0;\
      ((HLH_strgen_dyn_array *)(array))->data = HLH_STRGEN_MALLOC(sizeof(type)*(((HLH_strgen_dyn_array *)(array))->size));\
      HLH_ERROR_CHECK(((HLH_strgen_dyn_array *)(array))->data!=NULL,0x001);\
   }while(0)

#define HLH_dyn_array_free(type, array) \
   do\
   {\
      if(((HLH_strgen_dyn_array *)(array))->data!=NULL)\
      {\
         HLH_STRGEN_FREE(((HLH_strgen_dyn_array *)(array))->data);\
         ((HLH_strgen_dyn_array *)(array))->data = NULL;\
         ((HLH_strgen_dyn_array *)(array))->used = 0;\
         ((HLH_strgen_dyn_array *)(array))->size = 0;\
      }\
   }while(0)

#define HLH_dyn_array_add(type, array, grow, element) \
   do\
   {  \
      void *HLH_strgen_mem_old = ((HLH_strgen_dyn_array *)(array))->data;\
      if(((HLH_strgen_dyn_array *)(array))->used==((HLH_strgen_dyn_array *)(array))->size)\
      {\
         ((HLH_strgen_dyn_array *)(array))->size+=grow;\
         ((HLH_strgen_dyn_array *)(array))->data = HLH_STRGEN_REALLOC(((HLH_strgen_dyn_array *)(array))->data,sizeof(type)*(((HLH_strgen_dyn_array *)(array))->size));\
      }\
      if(((HLH_strgen_dyn_array *)(array))->data==NULL)\
      {\
         ((HLH_strgen_dyn_array *)(array))->size-=grow;\
         ((HLH_strgen_dyn_array *)(array))->data = HLH_strgen_mem_old;\
         HLH_ERROR_CHECK(0,0x002);\
      }\
      else\
      {\
         ((type *)((HLH_strgen_dyn_array *)(array)->data))[((HLH_strgen_dyn_array *)(array))->used] = (element);\
         ((HLH_strgen_dyn_array *)(array))->used++;\
      }\
   }while(0)

#define HLH_dyn_array_element(type, array, index) \
   (((type *)((HLH_strgen_dyn_array *)(array)->data))[index])

#define HLH_READ(v,m,p,l,t) \
   do { HLH_ERROR_CHECK((p)+sizeof(t)<=(l),0x200); (v) = (*((t *)((m)+(p)))); (p)+=sizeof(t); } while(0)

#define HLH_EXPAND 128
#define HLH_SIZE 128

static int HLH_strgen_entry_add(HLH_strgen *str, HLH_strgen_dyn_array prefix, char suffix, int start);
static int HLH_strgen_read_line(FILE *f, HLH_strgen_dyn_array *line);
static char HLH_strgen_prefix_choose_random(HLH_strgen_dyn_array suffixes);
static int HLH_strgen_prefix_find(HLH_strgen *str, HLH_strgen_dyn_array prefix);

typedef struct
{
   HLH_strgen_dyn_array prefix;
   HLH_strgen_dyn_array suffix;
}HLH_strgen_entry;

typedef struct
{
   char entry;
   uint32_t weight;
}HLH_strgen_suffix;

HLH_strgen *HLH_strgen_new(uint32_t order)
{
   HLH_strgen *str = NULL;

   HLH_ERROR_CHECK(order!=0,0x100);

   str = HLH_STRGEN_MALLOC(sizeof(*str));
   HLH_ERROR_CHECK(str!=NULL,0x001);
   memset(str,0,sizeof(*str));

   str->order = order;
   HLH_dyn_array_init(HLH_strgen_entry,&str->chain,HLH_SIZE);
   HLH_dyn_array_init(uint32_t,&str->chain_start,HLH_SIZE);

   return str;

HLH_err:

   if(str!=NULL)
   {
      HLH_dyn_array_free(HLH_strgen_entry,&str->chain);
      HLH_dyn_array_free(uint32_t,&str->chain_start);

      HLH_STRGEN_FREE(str);
   }

   return NULL;
}

int HLH_strgen_destroy(HLH_strgen *str)
{
   unsigned i = 0;

   HLH_ERROR_CHECK(str!=NULL,0x101);

   for(i = 0;i<str->chain.used;i++)
   {
      HLH_strgen_entry *e = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,i);
      HLH_dyn_array_free(char,&e->prefix);
      HLH_dyn_array_free(HLH_strgen_suffix,&e->suffix);
   }

   HLH_dyn_array_free(HLH_strgen_entry,&str->chain);
   HLH_dyn_array_free(uint32_t,&str->chain_start);
   HLH_STRGEN_FREE(str);

   return 0;

HLH_err:

   return 1;
}

int HLH_strgen_add_file(HLH_strgen *str, FILE *f)
{
   int status = 1;
   HLH_strgen_dyn_array line = {0};

   HLH_ERROR_CHECK(str!=NULL,0x101);
   HLH_ERROR_CHECK(f!=NULL,0x101);

   HLH_dyn_array_init(char,&line,HLH_SIZE);

   do
   {
      status = HLH_strgen_read_line(f,&line);

      HLH_ERROR_CHECK(!HLH_strgen_add_string(str,(char *)line.data),0x000);
   }
   while(status);

   HLH_dyn_array_free(char,&line);

   return 0;

HLH_err:

   if(line.data!=NULL)
      HLH_dyn_array_free(char,&line);

   return 1;
}

int HLH_strgen_add_string(HLH_strgen *str, const char *s)
{
   unsigned i = 0;
   unsigned j = 0;
   unsigned max = strlen(s)+1;
   HLH_strgen_dyn_array prefix = {0};

   HLH_ERROR_CHECK(str!=NULL,0x101);
   HLH_ERROR_CHECK(s!=NULL,0x101);
   HLH_ERROR_CHECK(max>=str->order,0x100);

   for(i = 0;i<max;i++)
   {
      char suffix = '\0';

      if(i+str->order+1>max)
         continue;

      HLH_dyn_array_init(char,&prefix,str->order);

      for(j = i;j<i+str->order;j++)
         HLH_dyn_array_add(char,&prefix,HLH_EXPAND,s[j]);

      if(i+str->order<max)
         suffix = s[i+str->order];

      HLH_strgen_entry_add(str,prefix,suffix,i==0);

      HLH_dyn_array_free(char,&prefix);
      prefix.data = NULL;
   }

   return 0;

HLH_err:

   if(prefix.data!=NULL)
      HLH_dyn_array_free(char,&prefix);

   return 1;
}

static int HLH_strgen_entry_add(HLH_strgen *str, HLH_strgen_dyn_array prefix, char suffix, int start)
{
   unsigned i = 0;
   unsigned j = 0;
   HLH_strgen_entry entry = {0};

   HLH_ERROR_CHECK(str!=NULL,0x101);

   for(i = 0;i<str->chain.used;i++)
   {
      HLH_strgen_entry *entry_p = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,i);
      int found = 1;

      for(j = 0;j<prefix.used;j++)
      {
         if(HLH_dyn_array_element(char,&prefix,j)!=HLH_dyn_array_element(char,&entry_p->prefix,j))
         {
            found = 0;
            break;
         }
      }

      if(found)
      {
         found = 0;

         for(j = 0;j<entry_p->suffix.used;j++)
         {
            HLH_strgen_suffix *s = &HLH_dyn_array_element(HLH_strgen_suffix,&entry_p->suffix,j);

            if(s->entry==suffix)
            {
               found = 1;
               s->weight++;
               break;
            }
         }

         if(!found)
         {
            HLH_strgen_suffix suff = {.entry = suffix, .weight = 1};

            HLH_dyn_array_add(HLH_strgen_suffix,&entry_p->suffix,HLH_EXPAND,suff);
         }

         return 0;
      }
   }

   HLH_dyn_array_init(char,&entry.prefix,HLH_SIZE);
   for(i = 0;i<prefix.used;i++)
      HLH_dyn_array_add(char,&entry.prefix,HLH_EXPAND,HLH_dyn_array_element(char,&prefix,i));
   HLH_dyn_array_init(HLH_strgen_suffix,&entry.suffix,HLH_SIZE);
   HLH_strgen_suffix suff = {.entry = suffix, .weight = 1};
   HLH_dyn_array_add(HLH_strgen_suffix,&entry.suffix,HLH_SIZE,suff);

   //Set to NULL to prevent freeing on error
   HLH_dyn_array_add(HLH_strgen_entry,&str->chain,HLH_SIZE,entry);
   entry.prefix.data = NULL;
   entry.suffix.data = NULL;

   if(start)
      HLH_dyn_array_add(uint32_t,&str->chain_start,HLH_EXPAND,str->chain.used-1);

   return 0;

HLH_err:

   if(entry.prefix.data!=NULL)
      HLH_dyn_array_free(char,&entry.prefix);

   if(entry.suffix.data!=NULL)
      HLH_dyn_array_free(HLH_strgen_suffix,&entry.suffix);

   return 1;
}

static int HLH_strgen_read_line(FILE *f, HLH_strgen_dyn_array *line)
{
   char ch = 0;

   line->used = 0;
   ch = fgetc(f);

   if(ch==EOF)
      return 0;

   while(ch!='\n'&&ch!=EOF)
   {
      HLH_dyn_array_add(char,line,HLH_EXPAND,ch);
      ch = fgetc(f);
   }

   HLH_dyn_array_add(char,line,HLH_EXPAND,'\0');
   
   return 1;

HLH_err:

   return 0;
}

char *HLH_strgen_generate(HLH_strgen *str)
{
   char *last = NULL;
   unsigned i = 0;
   uint32_t prefix = 0;
   HLH_strgen_dyn_array last_arr = {0};
   HLH_strgen_dyn_array sentence = {0};
   HLH_strgen_entry *e = NULL;

   HLH_ERROR_CHECK(str!=NULL,0x101);
   HLH_ERROR_CHECK(str->chain_start.used!=0,0x100);

   HLH_dyn_array_init(char,&last_arr,str->order);
   last_arr.used = str->order;
   last = (char *)last_arr.data;
   HLH_dyn_array_init(char,&sentence,HLH_SIZE);

   //Generate starting words
   prefix = HLH_dyn_array_element(uint32_t,&str->chain_start,HLH_STRGEN_RAND()%str->chain_start.used);
   e = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,prefix);
   for(i = 0;i<e->prefix.used;i++)
   {
      last[i] = HLH_dyn_array_element(char,&e->prefix,i);
      HLH_dyn_array_add(char,&sentence,HLH_EXPAND,last[i]);
   }

   while(last[str->order-1]!='\0')
   {
      e = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,HLH_strgen_prefix_find(str,last_arr));
      prefix = HLH_strgen_prefix_choose_random(e->suffix);

      HLH_dyn_array_add(char,&sentence,HLH_EXPAND,prefix);

      for(i = 0;i<str->order-1;i++)
         last[i] = last[i+1];
      last[str->order-1] = prefix;
   }

   HLH_dyn_array_free(char,&last_arr);

   return (char *)sentence.data;

HLH_err:
   
   if(last_arr.data!=NULL)
      HLH_dyn_array_free(char,&last_arr);

   if(sentence.data!=NULL)
      HLH_dyn_array_free(char,&sentence);

   return NULL;
}

static int HLH_strgen_prefix_find(HLH_strgen *str, HLH_strgen_dyn_array prefix)
{
   unsigned i = 0;
   unsigned j = 0;

   //Should never happen, but check it anyways
   HLH_ERROR_CHECK(str!=NULL,0x000);

   for(i = 0;i<str->chain.used;i++)
   {
      HLH_strgen_entry *entry = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,i);
      int found = 1;

      for(j = 0;j<prefix.used;j++)
      {
         if(HLH_dyn_array_element(char,&prefix,j)!=HLH_dyn_array_element(char,&entry->prefix,j))
         {
            found = 0;
            break;
         }
      }

      if(found)
         return i;
   }

HLH_err:

   return -1;
}

static char HLH_strgen_prefix_choose_random(HLH_strgen_dyn_array suffixes)
{
   char result = 0;
   unsigned i = 0;
   unsigned j = 0;
   HLH_strgen_dyn_array entries = {0};

   HLH_ERROR_CHECK(suffixes.used!=0,0x100);

   HLH_dyn_array_init(char,&entries,HLH_SIZE);

   for(i = 0;i<suffixes.used;i++)
   {
      for(j = 0;j<HLH_dyn_array_element(HLH_strgen_suffix,&suffixes,i).weight;j++)
         HLH_dyn_array_add(char,&entries,HLH_EXPAND,HLH_dyn_array_element(HLH_strgen_suffix,&suffixes,i).entry);
   }

   result = HLH_dyn_array_element(char,&entries,HLH_STRGEN_RAND()%entries.used);
   HLH_dyn_array_free(char,&entries);

   return result;

HLH_err:

   if(entries.data!=NULL)
      HLH_dyn_array_free(char,&entries);

   return '\0';
}

//TODO: handle big/little endian
int HLH_strgen_model_save(const HLH_strgen *str, FILE *f)
{
   unsigned i = 0;
   unsigned j = 0;

   HLH_ERROR_CHECK(str!=NULL,0x101);
   HLH_ERROR_CHECK(f!=NULL,0x101);

   HLH_ERROR_CHECK(fwrite(&str->order,sizeof(uint32_t),1,f)==1,0x003);

   HLH_ERROR_CHECK(fwrite(&str->chain_start.used,sizeof(str->chain_start.used),1,f)==1,0x003);
   for(i = 0;i<str->chain_start.used;i++)
      HLH_ERROR_CHECK(fwrite(&HLH_dyn_array_element(uint32_t,&str->chain_start,i),sizeof(uint32_t),1,f)==1,0x003);

   HLH_ERROR_CHECK(fwrite(&str->chain.used,sizeof(str->chain.used),1,f)==1,0x003);
   for(i = 0;i<str->chain.used;i++)
   {
      const HLH_strgen_entry *e = &HLH_dyn_array_element(HLH_strgen_entry,&str->chain,i);

      HLH_ERROR_CHECK(fwrite(&e->prefix.used,sizeof(e->prefix.used),1,f)==1,0x003);
      for(j = 0;j<e->prefix.used;j++)
         HLH_ERROR_CHECK(fwrite(&HLH_dyn_array_element(char,&e->prefix,j),sizeof(char),1,f)==1,0x003);

      HLH_ERROR_CHECK(fwrite(&e->suffix.used,sizeof(e->suffix.used),1,f)==1,0x003);
      for(j = 0;j<e->suffix.used;j++)
      {
         const HLH_strgen_suffix *s = &HLH_dyn_array_element(HLH_strgen_suffix,&e->suffix,j);

         HLH_ERROR_CHECK(fwrite(&s->entry,sizeof(s->entry),1,f)==1,0x003);
         HLH_ERROR_CHECK(fwrite(&s->weight,sizeof(s->weight),1,f)==1,0x003);
      }
   }

   return 0;

HLH_err:

   return 1;
}

HLH_strgen *HLH_strgen_model_load(FILE *f)
{
   HLH_strgen *out = NULL;
   int size = 0;
   uint8_t *buffer = NULL;

   HLH_ERROR_CHECK(f!=NULL,0x101);

   HLH_ERROR_CHECK(fseek(f,0,SEEK_END)==0,0x004);
   size = ftell(f);
   HLH_ERROR_CHECK(size!=EOF,0x005);
   HLH_ERROR_CHECK(fseek(f,0,SEEK_SET)==0,0x004);

   buffer = HLH_STRGEN_MALLOC(size+1);
   HLH_ERROR_CHECK(buffer!=NULL,0x001);

   HLH_ERROR_CHECK(fread(buffer,size,1,f)==1,0x003);

   //Not necessary, I just like to ensure any buffer read from a
   //file is 0 terminated, be it binary or a string
   buffer[size] = '\0';

   out = HLH_strgen_model_load_mem(buffer,size);
   HLH_ERROR_CHECK(out!=NULL,0x000);

   HLH_STRGEN_FREE(buffer);

   return out;

HLH_err:

   if(buffer!=NULL)
      HLH_STRGEN_FREE(buffer);

   return NULL;
}

//TODO: handle big/little endian
HLH_strgen *HLH_strgen_model_load_mem(const uint8_t *data, unsigned length)
{
   unsigned i = 0;
   unsigned j = 0;
   unsigned pos = 0;
   uint32_t order = 0;
   uint32_t used = 0;
   uint32_t e_used = 0;
   HLH_strgen *str = NULL;
   HLH_strgen_entry e = {0};

   HLH_ERROR_CHECK(data!=NULL,0x101);
   HLH_ERROR_CHECK(length>0,0x100);

   HLH_READ(order,data,pos,length,uint32_t);
   str = HLH_strgen_new(order);
   HLH_ERROR_CHECK(str!=NULL,0x000);

   HLH_READ(used,data,pos,length,uint32_t);
   for(i = 0;i<used;i++)
   {
      uint32_t val = 0;

      HLH_READ(val,data,pos,length,uint32_t);
      HLH_dyn_array_add(uint32_t,&str->chain_start,HLH_EXPAND,val);
   }

   HLH_READ(used,data,pos,length,uint32_t);
   for(i = 0;i<used;i++)
   {
      e = (HLH_strgen_entry){0};
      e_used = 0;

      HLH_dyn_array_init(char,&e.prefix,HLH_SIZE);
      HLH_dyn_array_init(HLH_strgen_suffix,&e.suffix,HLH_SIZE);

      HLH_READ(e_used,data,pos,length,uint32_t); 
      for(j = 0;j<e_used;j++)
      {
         char val = 0;

         HLH_READ(val,data,pos,length,char);
         HLH_dyn_array_add(char,&e.prefix,HLH_EXPAND,val);
      }

      HLH_READ(e_used,data,pos,length,uint32_t); 
      for(j = 0;j<e_used;j++)
      {
         HLH_strgen_suffix s = {0};

         HLH_READ(s.entry,data,pos,length,char);
         HLH_READ(s.weight,data,pos,length,uint32_t);

         HLH_dyn_array_add(HLH_strgen_suffix,&e.suffix,HLH_EXPAND,s);
      }

      HLH_dyn_array_add(HLH_strgen_entry,&str->chain,HLH_EXPAND,e);
   }

   return str;

HLH_err:

   if(str!=NULL)
      HLH_strgen_destroy(str);

   if(e.prefix.data!=NULL)
      HLH_dyn_array_free(char,&e.prefix);

   if(e.suffix.data!=NULL)
      HLH_dyn_array_free(HLH_strgen_suffix,&e.suffix);

   return NULL;
}

#undef HLH_dyn_array_init
#undef HLH_dyn_array_free
#undef HLH_dyn_array_add
#undef HLH_dyn_array_element
#undef HLH_EXPAND
#undef HLH_SIZE
#undef HLH_READ

#endif
#endif
