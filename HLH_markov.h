#ifndef _HLH_MARKOV_H_

/*
   Markov chain string generator

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define HLH_MARKOV_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

/*
   malloc(), realloc(), free() and rand() can be overwritten by 
   defining the following macros:

   HLH_MARKOV_MALLOC
   HLH_MARKOV_FREE
   HLH_MARKOV_REALLOC
   HLH_MARKOV_RAND
*/

#define _HLH_MARKOV_H_

#include <stdint.h>

typedef struct HLH_markov_context_word HLH_markov_context_word;
typedef struct HLH_markov_context_char HLH_markov_context_char;
typedef struct HLH_markov_count HLH_markov_count;

typedef enum
{
   HLH_MARKOV_CHAR,
   HLH_MARKOV_WORD,
}HLH_markov_model_type;

//Dynamic array types
//I used to use macors for dynamic arrays, but
//a lot of the time you need just a little
//more flexibility, so I have seperate implementations instead
typedef struct
{
   char **data;
   uint32_t data_used;
   uint32_t data_size;
}HLH_markov_str_array;

typedef struct
{
   uint32_t *data;
   uint32_t data_used;
   uint32_t data_size;
}HLH_markov_u32_array;

typedef struct
{
   HLH_markov_context_word *data;
   uint32_t data_used;
   uint32_t data_size;
}HLH_markov_context_word_array;

typedef struct
{
   HLH_markov_context_char *data;
   uint32_t data_used;
   uint32_t data_size;
}HLH_markov_context_char_array;

typedef struct
{
   HLH_markov_count *data;
   uint32_t data_used;
   uint32_t data_size;
}HLH_markov_count_array;

typedef struct
{

}HLH_markov_model_char;

typedef struct
{
   HLH_markov_str_array words[256];
   HLH_markov_u32_array start_words;
   HLH_markov_context_word_array contexts[256];
}HLH_markov_model_word;

typedef struct
{
   HLH_markov_model_type type;
   union
   {
      HLH_markov_model_word mword;
      HLH_markov_model_char mchar;
   }as;
}HLH_markov_model;

HLH_markov_model *HLH_markov_model_new(HLH_markov_model_type type);
void HLH_markov_model_delete(HLH_markov_model *model);

void HLH_markov_model_add(HLH_markov_model *model, const char *str);

const char *HLH_markov_model_generate(const HLH_markov_model *model);

#endif

#ifdef HLH_MARKOV_IMPLEMENTATION
#ifndef HLH_MARKOV_IMPLEMENTATION_ONCE
#define HLH_MARKOV_IMPLEMENTATION_ONCE

#ifndef HLH_MARKOV_MALLOC
#define HLH_MARKOV_MALLOC malloc
#endif

#ifndef HLH_MARKOV_FREE
#define HLH_MARKOV_FREE free
#endif

#ifndef HLH_MARKOV_REALLOC
#define HLH_MARKOV_REALLOC realloc
#endif

#ifndef HLH_MARKOV_RAND
#define HLH_MARKOV_RAND rand
#endif

#ifndef HLH_MARKOV_ORDER
#define HLH_MARKOV_ORDER 3
#endif

#define HLH_FNV_32_PRIME ((uint32_t)0x01000193)

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

struct HLH_markov_context_word
{
   uint32_t context[HLH_MARKOV_ORDER];
   uint32_t context_size;
   uint32_t total;
   HLH_markov_count_array counts;
};

struct HLH_markov_count
{
   uint32_t count;
   uint32_t item;
};

static void _HLH_markov_model_add_char(HLH_markov_model *model, const char *str);
static void _HLH_markov_model_add_word(HLH_markov_model *model, const char *str);
static const char *_HLH_markov_model_generate_word(const HLH_markov_model *model);
static const char *_HLH_markov_model_generate_char(const HLH_markov_model *model);

static uint32_t _HLH_markov_model_word_add(HLH_markov_model *model, const char *word);

static HLH_markov_context_word *_HLH_markov_context_word_find_or_create(HLH_markov_context_word_array *array, uint32_t context[HLH_MARKOV_ORDER], uint32_t context_size);

static uint32_t _HLH_markov_fnv32a(const char *str);

static void     _HLH_markov_u32_array_add(HLH_markov_u32_array *array, uint32_t num);
static void     _HLH_markov_u32_array_free(HLH_markov_u32_array *array);
static uint32_t _HLH_markov_str_array_add(HLH_markov_str_array *array, const char *str);
static void     _HLH_markov_str_array_free(HLH_markov_str_array *array);
static void     _HLH_markov_count_array_add(HLH_markov_count_array *array, uint32_t item);
static void     _HLH_markov_count_array_free(HLH_markov_count_array *array);

HLH_markov_model *HLH_markov_model_new(HLH_markov_model_type type)
{
   HLH_markov_model *model = HLH_MARKOV_MALLOC(sizeof(*model));
   memset(model,0,sizeof(*model));
   model->type = type;

   return model;
}

void HLH_markov_model_delete(HLH_markov_model *model)
{
   //TODO
   free(model);
}

void HLH_markov_model_add(HLH_markov_model *model, const char *str)
{
   if(model->type==HLH_MARKOV_CHAR)
      _HLH_markov_model_add_char(model,str);
   else if(model->type==HLH_MARKOV_WORD)
      _HLH_markov_model_add_char(model,str);
}

const char *HLH_markov_model_generate(const HLH_markov_model *model)
{
   if(model->type==HLH_MARKOV_CHAR)
      _HLH_markov_model_generate_char(model);
   else if(model->type==HLH_MARKOV_WORD)
      _HLH_markov_model_generate_word(model);
}

static const char *_HLH_markov_model_generate_word(const HLH_markov_model *model)
{
   /*Uint32_array sentence = {0};
   uint32_array_add(&sentence,model->starting_words.data[rand()%model->starting_words.data_used]);

   int done = 0;
   while(!done)
   {
      int start = sentence.data_used;
#if MORE_RANDOM
      int backoff = (rand()%ORDER)+1;
#else
      int backoff = ORDER;
#endif
      if(start-backoff<0)
         backoff = start;
      start--;

      uint32_t context[ORDER] = {0};
      for(int i = 0;i<backoff;i++)
         context[i] = sentence.data[start-i];
      
      Context *model_context = NULL;
      for(int i = backoff;i>0;i--)
      {
         uint8_t xor = 0;
         for(int j = 0;j<i;j++)
         {
            uint32_t word = sentence.data[start-j];

            xor^=word&255;
            xor^=(word>>8)&255;
            xor^=(word>>16)&255;
            xor^=(word>>24)&255;
         }

         model_context = context_find(&model->contexts[xor],context,i);
         if(model_context!=NULL)
            break;
      }

      if(model_context!=NULL)
      {
#if WEIGHTED_RANDOM
         uint32_t num = rand()%model_context->total;
         uint32_t cur = 0;
         for(int i = 0;i<model_context->counts.data_used;i++)
         {
            cur+=model_context->counts.data[i].count;
            if(cur>=num)
            {
               uint32_array_add(&sentence,model_context->counts.data[i].word);
               break;
            }
         }
#else
         uint32_array_add(&sentence,model_context->counts.data[rand()%model_context->counts.data_used].word);
#endif
      }
      else
      {
         done = 1;
      }
   }

   for(int i = 0;i<sentence.data_used;i++)
      printf("%s ",markov_model_get_word(model,sentence.data[i]));
   puts("");*/
}

static const char *_HLH_markov_model_generate_char(const HLH_markov_model *model)
{
   //TODO
}

static void _HLH_markov_model_add_char(HLH_markov_model *model, const char *str)
{
   //TODO
}

static void _HLH_markov_model_add_word(HLH_markov_model *model, const char *str)
{
   char *str_line = malloc(sizeof(*str_line)*(strlen(str)+1));
   strcpy(str_line,str);
   HLH_markov_u32_array sentence = {0};

   //Collect words and convert sentence to integers
   const char *token = " ";
   char *word = strtok(str_line,token);
   int first = 1;
   while(word!=NULL)
   {
      uint32_t word_index = _HLH_markov_model_word_add(model,word);
      _HLH_markov_u32_array_add(&sentence,word_index);
      if(first)
      {
         first = 0;
         _HLH_markov_u32_array_add(&model->as.mword.start_words,word_index);
      }

      word = strtok(NULL,token);
   }

   //Analyze sentence
   for(int i = 0;i<sentence.data_used;i++)
   {
      uint32_t context[HLH_MARKOV_ORDER] = {0};
      uint32_t event = sentence.data[i];

      uint8_t xor = 0;
      for(int m = 1;m<=HLH_MARKOV_ORDER;m++)
      {
         if((int)i-m<0)
            break;

         context[m-1] = sentence.data[i-m];
         xor^=context[m-1]&255;
         xor^=(context[m-1]>>8)&255;
         xor^=(context[m-1]>>16)&255;
         xor^=(context[m-1]>>24)&255;

         HLH_markov_context_word *model_context = _HLH_markov_context_word_find_or_create(&model->as.mword.contexts[xor],context,m);
         model_context->total++;
         _HLH_markov_count_array_add(&model_context->counts,event);
      }
   }

   _HLH_markov_u32_array_free(&sentence);
}

static void _HLH_markov_u32_array_add(HLH_markov_u32_array *array, uint32_t num)
{
   if(array->data==NULL)
   {
      array->data_used = 0;
      array->data_size = 16;
      array->data = malloc(sizeof(*array->data)*array->data_size);
   }

   array->data[array->data_used++] = num;

   if(array->data_used>=array->data_size)
   {
      array->data_size+=16;
      array->data = realloc(array->data,sizeof(*array->data)*array->data_size);
   }
}

static void _HLH_markov_u32_array_free(HLH_markov_u32_array *array)
{
   if(array==NULL||array->data==NULL)
      return;

   free(array->data);
   array->data = NULL;
   array->data_used = 0;
   array->data_size = 0;
}

static uint32_t _HLH_markov_model_word_add(HLH_markov_model *model, const char *word)
{
   uint8_t index = _HLH_markov_fnv32a(word)&255;
   return _HLH_markov_str_array_add(&model->as.mword.words[index],word)|(index<<24);
}

static uint32_t _HLH_markov_str_array_add(HLH_markov_str_array *array, const char *str)
{
   if(array->data==NULL)
   {
      array->data_used = 0;
      array->data_size = 16;
      array->data = malloc(sizeof(*array->data)*array->data_size);
   }

   for(int i = 0;i<array->data_used;i++)
      if(strcmp(array->data[i],str)==0)
         return i;

   int len = strlen(str)+1;
   array->data[array->data_used] = malloc(sizeof(*array->data[array->data_used])*len);
   strcpy(array->data[array->data_used],str);
   array->data_used++;

   if(array->data_used>=array->data_size)
   {
      array->data_size+=16;
      array->data = realloc(array->data,sizeof(*array->data)*array->data_size);
   }
   
   return array->data_used-1;
}

static void _HLH_markov_str_array_free(HLH_markov_str_array *array)
{
   if(array==NULL||array->data==NULL)
      return;

   for(int i = 0;i<array->data_used;i++)
      free(array->data[i]);

   free(array->data);
   array->data = NULL;
   array->data_used = 0;
   array->data_size = 0;
}

static void _HLH_markov_count_array_add(HLH_markov_count_array *array, uint32_t item)
{
   if(array->data==NULL)
   {
      array->data_used = 0;
      array->data_size = 16;
      array->data = malloc(sizeof(*array->data)*array->data_size);
   }

   for(int i = 0;i<array->data_used;i++)
   {
      if(array->data[i].item==item)
      {
         array->data[i].count++;
         return;
      }
   }

   array->data[array->data_used].item = item;
   array->data[array->data_used].count = 1;
   array->data_used++;

   if(array->data_used>=array->data_size)
   {
      array->data_size+=16;
      array->data = realloc(array->data,sizeof(*array->data)*array->data_size);
   }
}

static void _HLH_markov_count_array_free(HLH_markov_count_array *array)
{
   if(array==NULL||array->data==NULL)
      return;

   free(array->data);
   array->data = NULL;
   array->data_used = 0;
   array->data_size = 0;
}

static HLH_markov_context_word *_HLH_markov_context_word_find_or_create(HLH_markov_context_word_array *array, uint32_t context[HLH_MARKOV_ORDER], uint32_t context_size)
{
   if(array->data==NULL)
   {
      array->data_used = 0;
      array->data_size = 16;
      array->data = malloc(sizeof(*array->data)*array->data_size);
   }

   for(int i = 0;i<array->data_used;i++)
   {
      if(array->data[i].context_size!=context_size)
         continue;
      if(memcmp(array->data[i].context,context,sizeof(context[0])*context_size)==0)
         return &array->data[i];
   }

   memcpy(array->data[array->data_used].context,context,sizeof(context[0])*context_size);
   array->data[array->data_used].context_size = context_size;
   array->data[array->data_used].total = 0;
   memset(&array->data[array->data_used].counts,0,sizeof(array->data[array->data_used].counts));
   array->data_used++;

   if(array->data_used>=array->data_size)
   {
      array->data_size+=16;
      array->data = realloc(array->data,sizeof(*array->data)*array->data_size);
   }
   
   return &array->data[array->data_used-1];
}

static uint32_t _HLH_markov_fnv32a(const char *str)
{
   uint32_t hval = 0x811c9dc5;
   unsigned char *s = (unsigned char *)str;
   while(*s) 
   {
      hval^=(uint32_t)*s++;
      hval *= HLH_FNV_32_PRIME;
   }

   return hval;
}

#undef HLH_FNV_32_PRIME 
#endif
#endif
