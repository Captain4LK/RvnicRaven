#ifndef _HLH_MATH_H_

/*
   32 bit floating point math 

   Written in 2021 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

/* 
   To create implementation (the function definitions) add
      #define HLH_MATH_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _HLH_MATH_H_

typedef struct { float x,y; }HLH_vec2;
typedef struct { float x,y,z; }HLH_vec3;
typedef struct { float x,y,z,w; }HLH_vec4;
typedef struct { float nums[4][4]; }HLH_mat4;

//Utility
float HLH_to_radians(float degrees);
float HLH_to_degrees(float radians);
float HLH_lerp(float a, float b, float t);
float HLH_sign(float f);
float HLH_min(float a, float b);
float HLH_max(float a, float b);
float HLH_clamp(float lo, float hi, float f);
float HLH_abs(float f);

//2d vector math
HLH_vec2 HLH_vector2(float x, float y);
HLH_vec2 HLH_add2(HLH_vec2 a, HLH_vec2 b);
HLH_vec2 HLH_add2_f(HLH_vec2 a, float f);
HLH_vec2 HLH_sub2(HLH_vec2 a, HLH_vec2 b);
HLH_vec2 HLH_sub2_f(HLH_vec2 a, float f);
HLH_vec2 HLH_mul2(HLH_vec2 a, HLH_vec2 b);
HLH_vec2 HLH_mul2_f(HLH_vec2 a, float f);
HLH_vec2 HLH_div2(HLH_vec2 a, HLH_vec2 b);
HLH_vec2 HLH_div2_f(HLH_vec2 a, float f);
float    HLH_dot2(HLH_vec2 a, HLH_vec2 b);
float    HLH_mag2(HLH_vec2 a);
float    HLH_magmag2(HLH_vec2 a);
HLH_vec2 HLH_norm2(HLH_vec2 a);
HLH_vec2 HLH_lerp2(HLH_vec2 a, HLH_vec2 b, float t);

//3d vector math
HLH_vec3 HLH_vector3(float x, float y, float z);
HLH_vec3 HLH_add3(HLH_vec3 a, HLH_vec3 b);
HLH_vec3 HLH_add3_f(HLH_vec3 a, float f);
HLH_vec3 HLH_sub3(HLH_vec3 a, HLH_vec3 b);
HLH_vec3 HLH_sub3_f(HLH_vec3 a, float f);
HLH_vec3 HLH_mul3(HLH_vec3 a, HLH_vec3 b);
HLH_vec3 HLH_mul3_f(HLH_vec3 a, float f);
HLH_vec3 HLH_div3(HLH_vec3 a, HLH_vec3 b);
HLH_vec3 HLH_div3_f(HLH_vec3 a, float f);
float    HLH_dot3(HLH_vec3 a, HLH_vec3 b);
float    HLH_mag3(HLH_vec3 a);
float    HLH_magmag3(HLH_vec3 a);
HLH_vec3 HLH_norm3(HLH_vec3 a);
HLH_vec3 HLH_cross3(HLH_vec3 a, HLH_vec3 b);
HLH_vec3 HLH_lerp3(HLH_vec3 a, HLH_vec3 b, float t);

//4d vector math
HLH_vec4 HLH_vector4(float x, float y, float z, float w);
HLH_vec4 HLH_add4(HLH_vec4 a, HLH_vec4 b);
HLH_vec4 HLH_add4_f(HLH_vec4 a, float f);
HLH_vec4 HLH_sub4(HLH_vec4 a, HLH_vec4 b);
HLH_vec4 HLH_sub4_f(HLH_vec4 a, float f);
HLH_vec4 HLH_mul4(HLH_vec4 a, HLH_vec4 b);
HLH_vec4 HLH_mul4_f(HLH_vec4 a, float f);
HLH_vec4 HLH_div4(HLH_vec4 a, HLH_vec4 b);
HLH_vec4 HLH_div4_f(HLH_vec4 a, float f);
float    HLH_dot4(HLH_vec4 a, HLH_vec4 b);
float    HLH_mag4(HLH_vec4 a);
float    HLH_magmag4(HLH_vec4 a);
HLH_vec4 HLH_norm4(HLH_vec4 a);
HLH_vec4 HLH_lerp4(HLH_vec4 a, HLH_vec4 b, float t);

//4x4 matrix math
HLH_mat4 HLH_ident4x4();
HLH_mat4 HLH_mul4x4(HLH_mat4 a, HLH_mat4 b);
HLH_mat4 HLH_scale4x4(HLH_vec3 v);
HLH_mat4 HLH_transpose4x4(HLH_mat4 a);
HLH_mat4 HLH_translate4x4(HLH_vec3 pos);
HLH_mat4 HLH_rotate4x4(HLH_vec3 axis, float angle);
HLH_mat4 HLH_perspective4x4(float fov, float aspect, float n, float f);
HLH_mat4 HLH_look_at4x4(HLH_vec3 eye, HLH_vec3 focus, HLH_vec3 up);

#endif

#ifdef HLH_MATH_IMPLEMENTATION
#ifndef HLH_MATH_IMPLEMENTATION_ONCE
#define HLH_MATH_IMPLEMENTATION_ONCE

#include <math.h>

float HLH_to_radians(float degrees)
{
   return degrees*(3.14159265359f/180.0f);
}

float HLH_to_degrees(float radians)
{
   return radians*(180.0f/3.14159265359f);
}

float HLH_lerp(float a, float b, float t)
{
   return (1.0f-t)*a+t*b;
}

float HLH_sign(float f)
{
   return f<0.0f?-1.0f:f>0.0f?1.0f:0.0f;
}

float HLH_min(float a, float b)
{
   return a<b?a:b;
}

float HLH_max(float a, float b)
{
   return a>b?a:b;
}

float HLH_clamp(float lo, float hi, float f)
{
   return HLH_min(hi,HLH_max(lo,f));
}

float HLH_abs(float f)
{
   if(f<0.0f)
      return -f;
   return f;
}

HLH_vec2 HLH_vector2(float x, float y)
{
   return (HLH_vec2){.x = x, .y = y};
}

HLH_vec2 HLH_add2(HLH_vec2 a, HLH_vec2 b)
{
   return HLH_vector2(a.x+b.x,a.y+b.y);
}

HLH_vec2 HLH_add2_f(HLH_vec2 a, float f)
{
   return HLH_vector2(a.x+f,a.y+f);
}

HLH_vec2 HLH_sub2(HLH_vec2 a, HLH_vec2 b)
{
   return HLH_vector2(a.x-b.x,a.y-b.y);
}

HLH_vec2 HLH_sub2_f(HLH_vec2 a, float f)
{
   return HLH_vector2(a.x-f,a.y-f);
}

HLH_vec2 HLH_mul2(HLH_vec2 a, HLH_vec2 b)
{
   return HLH_vector2(a.x*b.x,a.y*b.y);
}

HLH_vec2 HLH_mul2_f(HLH_vec2 a, float f)
{
   return HLH_vector2(a.x*f,a.y*f);
}

HLH_vec2 HLH_div2(HLH_vec2 a, HLH_vec2 b)
{
   return HLH_vector2(a.x/b.x,a.y/b.y);
}

HLH_vec2 HLH_div2_f(HLH_vec2 a, float f)
{
   return HLH_vector2(a.x/f,a.y/f);
}

float HLH_dot2(HLH_vec2 a, HLH_vec2 b)
{
   return a.x*b.x+a.y*b.y;
}

float HLH_mag2(HLH_vec2 a)
{
   return sqrtf(HLH_dot2(a,a));
}

float HLH_magmag2(HLH_vec2 a)
{
   return HLH_dot2(a,a);
}

HLH_vec2 HLH_norm2(HLH_vec2 a)
{
   return HLH_div2_f(a,HLH_mag2(a));
}

HLH_vec2 HLH_lerp2(HLH_vec2 a, HLH_vec2 b, float t)
{
   return HLH_vector2(HLH_lerp(a.x,b.x,t),
                      HLH_lerp(a.y,b.y,t));
}

HLH_vec3 HLH_vector3(float x, float y, float z)
{
   return (HLH_vec3){.x = x, .y = y, .z = z};
}

HLH_vec3 HLH_add3(HLH_vec3 a, HLH_vec3 b)
{
   return HLH_vector3(a.x+b.x,a.y+b.y,a.z+b.z);
}

HLH_vec3 HLH_add3_f(HLH_vec3 a, float f)
{
   return HLH_vector3(a.x+f,a.y+f,a.z+f);
}

HLH_vec3 HLH_sub3(HLH_vec3 a, HLH_vec3 b)
{
   return HLH_vector3(a.x-b.x,a.y-b.y,a.z-b.z);
}

HLH_vec3 HLH_sub3_f(HLH_vec3 a, float f)
{
   return HLH_vector3(a.x-f,a.y-f,a.z-f);
}

HLH_vec3 HLH_mul3(HLH_vec3 a, HLH_vec3 b)
{
   return HLH_vector3(a.x*b.x,a.y*b.y,a.z*b.z);
}

HLH_vec3 HLH_mul3_f(HLH_vec3 a, float f)
{
   return HLH_vector3(a.x*f,a.y*f,a.z*f);
}

HLH_vec3 HLH_div3(HLH_vec3 a, HLH_vec3 b)
{
   return HLH_vector3(a.x/b.x,a.y/b.y,a.z/b.z);
}

HLH_vec3 HLH_div3_f(HLH_vec3 a, float f)
{
   return HLH_vector3(a.x/f,a.y/f,a.z/f);
}

float HLH_dot3(HLH_vec3 a, HLH_vec3 b)
{
   return a.x*b.x+a.y*b.y+a.z*b.z;
}

float HLH_mag3(HLH_vec3 a)
{
   return sqrtf(HLH_dot3(a,a));
}

float HLH_magmag3(HLH_vec3 a)
{
   return HLH_dot3(a,a);
}

HLH_vec3 HLH_norm3(HLH_vec3 a)
{
   return HLH_div3_f(a,HLH_mag3(a));
}

HLH_vec3 HLH_cross3(HLH_vec3 a, HLH_vec3 b)
{
   return HLH_vector3(a.y*b.z-a.z*b.y,
                      a.z*b.x-a.x*b.z,
                      a.x*b.y-a.y*b.x);
}

HLH_vec3 HLH_lerp3(HLH_vec3 a, HLH_vec3 b, float t)
{
   return HLH_vector3(HLH_lerp(a.x,b.x,t),
                      HLH_lerp(a.y,b.y,t),
                      HLH_lerp(a.z,b.z,t));
}

HLH_vec4 HLH_vector4(float x, float y, float z, float w)
{
   return (HLH_vec4){.x = x, .y = y, .z = z, .w = w};
}

HLH_vec4 HLH_add4(HLH_vec4 a, HLH_vec4 b)
{
   return HLH_vector4(a.x+b.x,a.y+b.y,a.z+b.z,a.w+b.w);
}

HLH_vec4 HLH_add4_f(HLH_vec4 a, float f)
{
   return HLH_vector4(a.x+f,a.y+f,a.z+f,a.w+f);
}

HLH_vec4 HLH_sub4(HLH_vec4 a, HLH_vec4 b)
{
   return HLH_vector4(a.x-b.x,a.y-b.y,a.z-b.z,a.w-b.w);
}

HLH_vec4 HLH_sub4_f(HLH_vec4 a, float f)
{
   return HLH_vector4(a.x-f,a.y-f,a.z-f,a.w-f);
}

HLH_vec4 HLH_mul4(HLH_vec4 a, HLH_vec4 b)
{
   return HLH_vector4(a.x*b.x,a.y*b.y,a.z*b.z,a.w*b.w);
}

HLH_vec4 HLH_mul4_f(HLH_vec4 a, float f)
{
   return HLH_vector4(a.x*f,a.y*f,a.z*f,a.w*f);
}

HLH_vec4 HLH_div4(HLH_vec4 a, HLH_vec4 b)
{
   return HLH_vector4(a.x/b.x,a.y/b.y,a.z/b.z,a.w/b.w);
}

HLH_vec4 HLH_div4_f(HLH_vec4 a, float f)
{
   return HLH_vector4(a.x/f,a.y/f,a.z/f,a.w/f);
}

float HLH_dot4(HLH_vec4 a, HLH_vec4 b)
{
   return a.x*b.x+a.y*b.y+a.z*b.z+a.w*b.w;
}

float HLH_mag4(HLH_vec4 a)
{
   return sqrtf(HLH_dot4(a,a));
}

float HLH_magmag4(HLH_vec4 a)
{
   return HLH_dot4(a,a);
}

HLH_vec4 HLH_norm4(HLH_vec4 a)
{
   return HLH_div4_f(a,HLH_mag4(a));
}

HLH_vec4 HLH_lerp4(HLH_vec4 a, HLH_vec4 b, float t)
{
   return HLH_vector4(HLH_lerp(a.x,b.x,t),
                      HLH_lerp(a.y,b.y,t),
                      HLH_lerp(a.z,b.z,t),
                      HLH_lerp(a.w,b.w,t));
}

HLH_mat4 HLH_ident4x4()
{
   return HLH_scale4x4(HLH_vector3(1.0f,1.0f,1.0f));
}

HLH_mat4 HLH_mul4x4(HLH_mat4 a, HLH_mat4 b)
{
   HLH_mat4 out = {0};
   int k,r,c;

   for(c = 0;c<4;c++)
   {
      for(r = 0;r<4;r++)
      {
         out.nums[c][r] = 0.0f;
         for(k = 0;k<4;k++)
            out.nums[c][r]+=a.nums[k][r]*b.nums[c][k];
      }
   }

   return out;
}

HLH_mat4 HLH_scale4x4(HLH_vec3 v)
{
   HLH_mat4 out = {0};

   out.nums[0][0] = v.x;
   out.nums[1][1] = v.y;
   out.nums[2][2] = v.z;
   out.nums[3][3] = 1.0;

   return out;
}

HLH_mat4 HLH_transpose4x4(HLH_mat4 a)
{
   HLH_mat4 out;

   for(int c = 0;c<4;c++)
      for(int r = 0;r<4;r++)
         out.nums[r][c] = a.nums[c][r];

   return out;
}

HLH_mat4 HLH_translate4x4(HLH_vec3 pos)
{
   HLH_mat4 out = HLH_ident4x4();

   out.nums[3][0] = pos.x;
   out.nums[3][1] = pos.y;
   out.nums[3][2] = pos.z;

   return out;
}

HLH_mat4 HLH_rotate4x4(HLH_vec3 axis, float angle)
{
   HLH_mat4 out = HLH_ident4x4();
   float c,s;
   HLH_vec3 t;

   c = cosf(angle);
   s = sinf(angle);

   axis = HLH_norm3(axis);
   t = HLH_mul3_f(axis,1.0f-c);

   out.nums[0][0] = c*t.x*axis.x;
   out.nums[0][1] = t.x*axis.y+s*axis.z;
   out.nums[0][2] = t.x*axis.z-s*axis.y;
   out.nums[0][3] = 0.0f;

   out.nums[1][0] = t.y*axis.x-s*axis.z;
   out.nums[1][1] = c+t.y*axis.y;
   out.nums[1][2] = t.y*axis.z+s*axis.x;
   out.nums[1][3] = 0.0f;

   out.nums[2][0] = t.z*axis.x+s*axis.y;
   out.nums[2][1] = t.z*axis.y-s*axis.x;
   out.nums[2][2] = c+t.z*axis.z;
   out.nums[2][3] = 0.0f;

   return out;
}

HLH_mat4 HLH_perspective4x4(float fov, float aspect, float n, float f)
{
   HLH_mat4 out = {0};

   float a = 1.0f/tanf(fov*0.5f);

   out.nums[0][0] = a/aspect;
   out.nums[0][1] = 0.0f;
   out.nums[0][2] = 0.0f;
   out.nums[0][3] = 0.0f;

   out.nums[1][0] = 0.0f;
   out.nums[1][1] = a;
   out.nums[1][2] = 0.0f;
   out.nums[1][3] = 0.0f;

   out.nums[2][0] = 0.0f;
   out.nums[2][1] = 0.0f;
   out.nums[2][2] = (f+n)/(n-f);
   out.nums[2][3] = -1.0f;

   out.nums[3][0] = 0.0f;
   out.nums[3][1] = 0.0f;
   out.nums[3][2] = ((2.0f*f*n)/(n-f));
   out.nums[3][3] = 0.0f;

   return out;
}

HLH_mat4 HLH_look_at4x4(HLH_vec3 eye, HLH_vec3 focus, HLH_vec3 up)
{
   HLH_mat4 out = {0};
   HLH_vec3 forward = HLH_norm3(HLH_sub3(eye,focus));
   HLH_vec3 right = HLH_cross3(HLH_norm3(up),forward);
   up = HLH_cross3(forward,right);

   out.nums[0][0] = right.x;
   out.nums[0][1] = right.y;
   out.nums[0][2] = right.z;
   out.nums[1][0] = up.x;
   out.nums[1][1] = up.y;
   out.nums[1][2] = up.z;
   out.nums[2][0] = forward.x;
   out.nums[2][1] = forward.y;
   out.nums[2][2] = forward.z;

   out.nums[3][0] = eye.x;
   out.nums[3][1] = eye.y;
   out.nums[3][2] = eye.z;

   return out;
}

#endif
#endif
