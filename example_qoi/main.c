/*
   QOI converter

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb_image.h"

#define CUTE_PNG_IMPLEMENTATION
#include "../external/cute_png.h"
//-------------------------------------

//Internal includes
#define HLH_STREAM_IMPLEMENTATION
#include "../HLH_stream.h"

#define HLH_QOI_IMPLEMENTATION
#include "../HLH_qoi.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
//-------------------------------------

//Function prototypes
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   HLH_qoi_image img = {0};
   img.data = stbi_load(argv[1],&img.width,&img.height,&img.channels,4);

   //Encode
   FILE *f = fopen("out.qoi","wb");
   HLH_rw rw = {0};
   HLH_rw_init_file(&rw,f);
   HLH_qoi_encode(&rw,&img);
   HLH_rw_close(&rw);
   fclose(f);

   //Decode and compare
   f = fopen("out.qoi","rb");
   HLH_rw_init_file(&rw,f);
   HLH_qoi_image *img_dec = HLH_qoi_decode(&rw);
   printf("%d\n",memcmp(img_dec->data,img.data,sizeof(*img.data)*img.width*img.height));
   cp_image_t cp_img;
   cp_img.w = img_dec->width;
   cp_img.h = img_dec->height;
   cp_img.pix = img_dec->data;
   cp_save_png("out.png",&cp_img);
   HLH_rw_close(&rw);
   fclose(f);
   
   return 0;
}
//-------------------------------------
