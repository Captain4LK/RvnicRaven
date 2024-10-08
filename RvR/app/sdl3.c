/*
RvnicRaven - input and app managment

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdlib.h>
#include <SDL3/SDL.h>
//-------------------------------------

//Internal includes
#include "RvR_config.h"
#include "RvR/RvR_log.h"
#include "RvR/RvR_math.h"
#include "RvR/RvR_malloc.h"
#include "RvR/RvR_palette.h"
#include "RvR/RvR_app.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
typedef struct
{
   SDL_Gamepad *gamepad;
   SDL_Haptic *haptic;
   SDL_JoystickID id;
   int connected;
   uint8_t new_button_state[RVR_PAD_MAX];
   uint8_t old_button_state[RVR_PAD_MAX];
}rvr_amepad;

static int rvr_core_running = 1;
static uint32_t rvr_core_frame = 0;
static int rvr_core_frametimes[32] = {0};

static SDL_Window *rvr_sdl_window;
static SDL_Renderer *rvr_renderer;
static SDL_Texture *rvr_layer_texture;
static float rvr_pixel_scale;
static int rvr_window_width;
static int rvr_window_height;
static int rvr_view_x;
static int rvr_view_y;
static int rvr_view_width;
static int rvr_view_height;
static float rvr_delta;
static uint8_t rvr_key_map[SDL_SCANCODE_COUNT];
static uint8_t rvr_mouse_map[6];
static uint8_t rvr_gamepad_map[SDL_GAMEPAD_BUTTON_COUNT];
static uint8_t rvr_new_key_state[RVR_KEY_MAX];
static uint8_t rvr_old_key_state[RVR_KEY_MAX];
static rvr_amepad rvr_gamepads[4];
static int rvr_mouse_x_rel;
static int rvr_mouse_y_rel;
static char *rvr_text_input;
static int rvr_text_input_active;
static unsigned rvr_text_input_max;
static int rvr_mouse_x;
static int rvr_mouse_y;
static int rvr_mouse_wheel;
static int rvr_key_repeat = 0;

//Timing
static uint64_t rvr_frametime;
static uint64_t rvr_framedelay;
static uint64_t rvr_framestart;

static uint8_t *rvr_framebuffer = NULL;

static void rvr_update_viewport();
static int rvr_get_gamepad_index(int which);
//-------------------------------------

//Function prototypes

static void rvr_update_viewport();
static int rvr_get_gamepad_index(int which);
//-------------------------------------

//Function implementations

void RvR_quit()
{
   rvr_core_running = 0;
}

void RvR_init(char *title, int scale)
{
   Uint32 flags =
#ifndef __EMSCRIPTEN__
      SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#else
      SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#endif

   if(!SDL_Init(flags))
   {
      RvR_log_line("SDL_Init ", "%s\n", SDL_GetError());
      exit(-1);
   }

   if(scale==0)
   {
      SDL_Rect max_size;

      int count = 0;
      SDL_DisplayID *displays = SDL_GetDisplays(&count);
      SDL_DisplayID id = 0;
      if(count>0)
         id = displays[0];
      if(!SDL_GetDisplayUsableBounds(id, &max_size))
      {
         RvR_log_line("SDL_GetDisplayUsableBounds ", "%s\n", SDL_GetError());
      }
      else
      {
         int max_x, max_y;

         max_x = max_size.w / RVR_XRES;
         max_y = max_size.h / RVR_YRES;

         scale = (max_x>max_y)?max_y:max_x;
      }
      if(displays!=NULL)
         SDL_free(displays);
   }

   if(scale<=0)
      scale = 1;

   rvr_sdl_window = SDL_CreateWindow(title, RVR_XRES * scale, RVR_YRES * scale, SDL_WINDOW_RESIZABLE);
   if(rvr_sdl_window==NULL)
   {
      RvR_log_line("SDL_CreateWindow ", "%s\n", SDL_GetError());
      exit(-1);
   }

   rvr_renderer = SDL_CreateRenderer(rvr_sdl_window, NULL);
   if(rvr_renderer==NULL)
   {
      RvR_log_line("SDL_CreateRenderer ", "%s\n", SDL_GetError());
      exit(-1);
   }

   if(!SDL_SetRenderDrawColor(rvr_renderer, 0, 0, 0, 0))
      RvR_log_line("SDL_SetRenderDrawColor ", "%s\n", SDL_GetError());

   rvr_layer_texture = SDL_CreateTexture(rvr_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, RVR_XRES, RVR_YRES);
   if(rvr_layer_texture==NULL)
      RvR_log_line("SDL_CreateTexture ", "%s\n", SDL_GetError());

   if(!SDL_SetTextureBlendMode(rvr_layer_texture, SDL_BLENDMODE_BLEND))
      RvR_log_line("SDL_SetTextureBlendMode ", "%s\n", SDL_GetError());
   if(!SDL_SetTextureScaleMode(rvr_layer_texture,SDL_SCALEMODE_NEAREST))
      RvR_log_line("SDL_SetTextureScaleMode", "%s\n", SDL_GetError());

   rvr_update_viewport();

   rvr_key_map[0x00] = RVR_KEY_NONE;
   rvr_key_map[SDL_SCANCODE_A] = RVR_KEY_A;
   rvr_key_map[SDL_SCANCODE_B] = RVR_KEY_B;
   rvr_key_map[SDL_SCANCODE_C] = RVR_KEY_C;
   rvr_key_map[SDL_SCANCODE_D] = RVR_KEY_D;
   rvr_key_map[SDL_SCANCODE_E] = RVR_KEY_E;
   rvr_key_map[SDL_SCANCODE_F] = RVR_KEY_F;
   rvr_key_map[SDL_SCANCODE_G] = RVR_KEY_G;
   rvr_key_map[SDL_SCANCODE_H] = RVR_KEY_H;
   rvr_key_map[SDL_SCANCODE_I] = RVR_KEY_I;
   rvr_key_map[SDL_SCANCODE_J] = RVR_KEY_J;
   rvr_key_map[SDL_SCANCODE_K] = RVR_KEY_K;
   rvr_key_map[SDL_SCANCODE_L] = RVR_KEY_L;
   rvr_key_map[SDL_SCANCODE_M] = RVR_KEY_M;
   rvr_key_map[SDL_SCANCODE_N] = RVR_KEY_N;
   rvr_key_map[SDL_SCANCODE_O] = RVR_KEY_O;
   rvr_key_map[SDL_SCANCODE_P] = RVR_KEY_P;
   rvr_key_map[SDL_SCANCODE_Q] = RVR_KEY_Q;
   rvr_key_map[SDL_SCANCODE_R] = RVR_KEY_R;
   rvr_key_map[SDL_SCANCODE_S] = RVR_KEY_S;
   rvr_key_map[SDL_SCANCODE_T] = RVR_KEY_T;
   rvr_key_map[SDL_SCANCODE_U] = RVR_KEY_U;
   rvr_key_map[SDL_SCANCODE_V] = RVR_KEY_V;
   rvr_key_map[SDL_SCANCODE_W] = RVR_KEY_W;
   rvr_key_map[SDL_SCANCODE_X] = RVR_KEY_X;
   rvr_key_map[SDL_SCANCODE_Y] = RVR_KEY_Y;
   rvr_key_map[SDL_SCANCODE_Z] = RVR_KEY_Z;

   rvr_key_map[SDL_SCANCODE_F1] = RVR_KEY_F1;
   rvr_key_map[SDL_SCANCODE_F2] = RVR_KEY_F2;
   rvr_key_map[SDL_SCANCODE_F3] = RVR_KEY_F3;
   rvr_key_map[SDL_SCANCODE_F4] = RVR_KEY_F4;
   rvr_key_map[SDL_SCANCODE_F5] = RVR_KEY_F5;
   rvr_key_map[SDL_SCANCODE_F6] = RVR_KEY_F6;
   rvr_key_map[SDL_SCANCODE_F7] = RVR_KEY_F7;
   rvr_key_map[SDL_SCANCODE_F8] = RVR_KEY_F8;
   rvr_key_map[SDL_SCANCODE_F9] = RVR_KEY_F9;
   rvr_key_map[SDL_SCANCODE_F10] = RVR_KEY_F10;
   rvr_key_map[SDL_SCANCODE_F11] = RVR_KEY_F11;
   rvr_key_map[SDL_SCANCODE_F12] = RVR_KEY_F12;

   rvr_key_map[SDL_SCANCODE_DOWN] = RVR_KEY_DOWN;
   rvr_key_map[SDL_SCANCODE_LEFT] = RVR_KEY_LEFT;
   rvr_key_map[SDL_SCANCODE_RIGHT] = RVR_KEY_RIGHT;
   rvr_key_map[SDL_SCANCODE_UP] = RVR_KEY_UP;
   rvr_key_map[SDL_SCANCODE_RETURN] = RVR_KEY_ENTER;

   rvr_key_map[SDL_SCANCODE_BACKSPACE] = RVR_KEY_BACK;
   rvr_key_map[SDL_SCANCODE_ESCAPE] = RVR_KEY_ESCAPE;
   rvr_key_map[SDL_SCANCODE_TAB] = RVR_KEY_TAB;
   rvr_key_map[SDL_SCANCODE_DELETE] = RVR_KEY_DEL;
   rvr_key_map[SDL_SCANCODE_HOME] = RVR_KEY_HOME;
   rvr_key_map[SDL_SCANCODE_END] = RVR_KEY_END;
   rvr_key_map[SDL_SCANCODE_PAGEUP] = RVR_KEY_PGUP;
   rvr_key_map[SDL_SCANCODE_PAGEDOWN] = RVR_KEY_PGDN;
   rvr_key_map[SDL_SCANCODE_INSERT] = RVR_KEY_INS;
   rvr_key_map[SDL_SCANCODE_LSHIFT] = RVR_KEY_LSHIFT;
   rvr_key_map[SDL_SCANCODE_RSHIFT] = RVR_KEY_RSHIFT;
   rvr_key_map[SDL_SCANCODE_LCTRL] = RVR_KEY_LCTRL;
   rvr_key_map[SDL_SCANCODE_RCTRL] = RVR_KEY_RCTRL;
   rvr_key_map[SDL_SCANCODE_RALT] = RVR_KEY_RALT;
   rvr_key_map[SDL_SCANCODE_LALT] = RVR_KEY_LALT;
   rvr_key_map[SDL_SCANCODE_SPACE] = RVR_KEY_SPACE;

   rvr_key_map[SDL_SCANCODE_0] = RVR_KEY_0;
   rvr_key_map[SDL_SCANCODE_1] = RVR_KEY_1;
   rvr_key_map[SDL_SCANCODE_2] = RVR_KEY_2;
   rvr_key_map[SDL_SCANCODE_3] = RVR_KEY_3;
   rvr_key_map[SDL_SCANCODE_4] = RVR_KEY_4;
   rvr_key_map[SDL_SCANCODE_5] = RVR_KEY_5;
   rvr_key_map[SDL_SCANCODE_6] = RVR_KEY_6;
   rvr_key_map[SDL_SCANCODE_7] = RVR_KEY_7;
   rvr_key_map[SDL_SCANCODE_8] = RVR_KEY_8;
   rvr_key_map[SDL_SCANCODE_9] = RVR_KEY_9;

   rvr_key_map[SDL_SCANCODE_COMMA] = RVR_KEY_COMMA;
   rvr_key_map[SDL_SCANCODE_PERIOD] = RVR_KEY_PERIOD;

   rvr_key_map[SDL_SCANCODE_KP_0] = RVR_KEY_NP0;
   rvr_key_map[SDL_SCANCODE_KP_1] = RVR_KEY_NP1;
   rvr_key_map[SDL_SCANCODE_KP_2] = RVR_KEY_NP2;
   rvr_key_map[SDL_SCANCODE_KP_3] = RVR_KEY_NP3;
   rvr_key_map[SDL_SCANCODE_KP_4] = RVR_KEY_NP4;
   rvr_key_map[SDL_SCANCODE_KP_5] = RVR_KEY_NP5;
   rvr_key_map[SDL_SCANCODE_KP_6] = RVR_KEY_NP6;
   rvr_key_map[SDL_SCANCODE_KP_7] = RVR_KEY_NP7;
   rvr_key_map[SDL_SCANCODE_KP_8] = RVR_KEY_NP8;
   rvr_key_map[SDL_SCANCODE_KP_9] = RVR_KEY_NP9;
   rvr_key_map[SDL_SCANCODE_KP_MULTIPLY] = RVR_KEY_NP_MUL;
   rvr_key_map[SDL_SCANCODE_KP_PLUS] = RVR_KEY_NP_ADD;
   rvr_key_map[SDL_SCANCODE_KP_DIVIDE] = RVR_KEY_NP_DIV;
   rvr_key_map[SDL_SCANCODE_KP_MINUS] = RVR_KEY_NP_SUB;
   rvr_key_map[SDL_SCANCODE_KP_PERIOD] = RVR_KEY_NP_DECIMAL;
   rvr_key_map[SDL_SCANCODE_KP_ENTER] = RVR_KEY_NP_ENTER;

   rvr_mouse_map[SDL_BUTTON_LEFT] = RVR_BUTTON_LEFT;
   rvr_mouse_map[SDL_BUTTON_RIGHT] = RVR_BUTTON_RIGHT;
   rvr_mouse_map[SDL_BUTTON_MIDDLE] = RVR_BUTTON_MIDDLE;
   rvr_mouse_map[SDL_BUTTON_X1] = RVR_BUTTON_X1;
   rvr_mouse_map[SDL_BUTTON_X2] = RVR_BUTTON_X2;

   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_SOUTH] = RVR_PAD_A;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_EAST] = RVR_PAD_B;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_WEST] = RVR_PAD_X;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_NORTH] = RVR_PAD_Y;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_BACK] = RVR_PAD_BACK;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_GUIDE] = RVR_PAD_GUIDE;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_START] = RVR_PAD_START;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_LEFT_STICK] = RVR_PAD_LEFTSTICK;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_RIGHT_STICK] = RVR_PAD_RIGHTSTICK;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_LEFT_SHOULDER] = RVR_PAD_LEFTSHOULDER;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER] = RVR_PAD_RIGHTSHOULDER;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_DPAD_UP] = RVR_PAD_UP;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_DPAD_DOWN] = RVR_PAD_DOWN;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_DPAD_LEFT] = RVR_PAD_LEFT;
   rvr_gamepad_map[SDL_GAMEPAD_BUTTON_DPAD_RIGHT] = RVR_PAD_RIGHT;

   //Clear key states, just in case,
   //should already be empty since known at compile time
   memset(rvr_new_key_state, 0, sizeof(rvr_new_key_state));
   memset(rvr_old_key_state, 0, sizeof(rvr_old_key_state));
   for(int i = 0; i<4; i++)
   {
      memset(rvr_gamepads[i].new_button_state, 0, sizeof(rvr_gamepads[i].new_button_state));
      memset(rvr_gamepads[i].old_button_state, 0, sizeof(rvr_gamepads[i].old_button_state));
   }

   int fps = RvR_max(1, RvR_min(1000, RVR_FPS));
   rvr_framedelay = SDL_GetPerformanceFrequency() / fps;

   rvr_framebuffer = RvR_malloc(RVR_XRES * RVR_YRES, "RvR framebuffer");
   memset(rvr_framebuffer, 0, RVR_XRES * RVR_YRES);
}

void RvR_mouse_relative(int relative)
{
   if(!SDL_SetWindowRelativeMouseMode(rvr_sdl_window,relative))
      RvR_log_line("SDL_SetWindowRelativeMouseMode ", "%s\n", SDL_GetError());
   //if(!SDL_SetRelativeMouseMode(relative))
      //RvR_log_line("SDL_SetRelativeMouseMode ", "%s\n", SDL_GetError());
   //if(!SDL_SetRelativeMouseMode(relative))
      //RvR_log_line("SDL_SetRelativeMouseMode ", "%s\n", SDL_GetError());
}

void RvR_mouse_show(int show)
{
   if(show)
   {
      if(!SDL_ShowCursor())
         RvR_log_line("SDL_ShowCursor ", "%s\n", SDL_GetError());
   }
   else
   {
      if(!SDL_HideCursor())
         RvR_log_line("SDL_HideCursor ", "%s\n", SDL_GetError());
   }
   //if(!SDL_ShowCursor(show?SDL_ENABLE:SDL_DISABLE))
      //RvR_log_line("SDL_ShowCursor ", "%s\n", SDL_GetError());
}

void RvR_key_repeat(int repeat)
{
   rvr_key_repeat = repeat;
}

int RvR_running()
{
   return rvr_core_running;
}

void RvR_update()
{
   rvr_core_frame++;
   rvr_core_frametimes[rvr_core_frame & 31] = RvR_frametime();

   #ifndef __EMSCRIPTEN__
   rvr_frametime = SDL_GetPerformanceCounter() - rvr_framestart;

   if(rvr_framedelay>rvr_frametime)
   {
      SDL_Delay((uint32_t)(((rvr_framedelay - rvr_frametime) * 1000) / SDL_GetPerformanceFrequency()));
   }

   rvr_delta = (float)(SDL_GetPerformanceCounter() - rvr_framestart) / (float)SDL_GetPerformanceFrequency();
   rvr_framestart = SDL_GetPerformanceCounter();

#endif

   rvr_mouse_wheel = 0;
   memcpy(rvr_old_key_state, rvr_new_key_state, sizeof(rvr_new_key_state));
   for(int i = 0; i<4; i++)
      memcpy(rvr_gamepads[i].old_button_state, rvr_gamepads[i].new_button_state, sizeof(rvr_gamepads[0].new_button_state));

   //Event managing
   SDL_Event event;
   while(SDL_PollEvent(&event))
   {
      switch(event.type)
      {
      case SDL_EVENT_QUIT:
         RvR_quit();
         break;
      case SDL_EVENT_KEY_DOWN:
         if(rvr_text_input_active&&event.key.key==SDLK_BACKSPACE&&rvr_text_input[0]!='\0')
            rvr_text_input[strlen(rvr_text_input) - 1] = '\0';
         if(event.key.down)
         {
            rvr_new_key_state[rvr_key_map[event.key.scancode]] = 1;
            if(event.key.repeat&&rvr_key_repeat)
               rvr_old_key_state[rvr_key_map[event.key.scancode]] = 0;
         }
         break;
      case SDL_EVENT_KEY_UP:
         if(!event.key.down)
            rvr_new_key_state[rvr_key_map[event.key.scancode]] = 0;
         break;
      case SDL_EVENT_MOUSE_BUTTON_DOWN:
         if(event.button.down)
         {
            if(event.button.button==SDL_BUTTON_LEFT&&event.button.clicks==2)
               rvr_new_key_state[RVR_BUTTON_DBLECLICK] = 1;
            rvr_new_key_state[rvr_mouse_map[event.button.button]] = 1;
         }
         break;
      case SDL_EVENT_MOUSE_BUTTON_UP:
         if(!event.button.down)
         {
            if(event.button.button==SDL_BUTTON_LEFT)
               rvr_new_key_state[RVR_BUTTON_DBLECLICK] = 0;
            rvr_new_key_state[rvr_mouse_map[event.button.button]] = 0;
         }
         break;
      case SDL_EVENT_TEXT_INPUT:
         if(rvr_text_input_active&&strlen(rvr_text_input) + strlen(event.text.text)<rvr_text_input_max)
            strcat(rvr_text_input, event.text.text);
         break;
      case SDL_EVENT_MOUSE_WHEEL:
         rvr_mouse_wheel += (int)event.wheel.y;
         break;
      case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
         if(event.gbutton.down)
         {
            int id = rvr_get_gamepad_index(event.gbutton.which);
            rvr_gamepads[id].new_button_state[rvr_gamepad_map[event.gbutton.button]] = 1;
         }
         break;
      case SDL_EVENT_GAMEPAD_BUTTON_UP:
         if(!event.gbutton.down)
         {
            int id = rvr_get_gamepad_index(event.gbutton.which);
            rvr_gamepads[id].new_button_state[rvr_gamepad_map[event.gbutton.button]] = 0;
         }
         break;
      case SDL_EVENT_GAMEPAD_ADDED:
      {
         int which = event.cdevice.which;
         if(which<4)
         {
            rvr_gamepads[which].gamepad = SDL_OpenGamepad(which);
            rvr_gamepads[which].connected = 1;
            SDL_Joystick *j = SDL_GetGamepadJoystick(rvr_gamepads[which].gamepad);
            rvr_gamepads[which].id = SDL_GetJoystickID(j);
         }
      }
      break;
      case SDL_EVENT_GAMEPAD_REMOVED:
      {
         int which = event.cdevice.which;
         if(which<0)
            break;
         int id = rvr_get_gamepad_index(which);
         rvr_gamepads[id].connected = 0;
         SDL_CloseGamepad(rvr_gamepads[id].gamepad);
      }
      break;
      case SDL_EVENT_WINDOW_SHOWN:
      case SDL_EVENT_WINDOW_EXPOSED:
      case SDL_EVENT_WINDOW_RESIZED:
      case SDL_EVENT_WINDOW_MAXIMIZED:
      case SDL_EVENT_WINDOW_RESTORED:
         rvr_update_viewport();
         break;
      }
   }
   //-------------------------------------------

   float x, y;
   SDL_GetMouseState(&x, &y);

   x -= (float)rvr_view_x;
   y -= (float)rvr_view_y;
   rvr_mouse_x = (int)(x / rvr_pixel_scale);
   rvr_mouse_y = (int)(y / rvr_pixel_scale);

   float rx, ry;
   SDL_GetRelativeMouseState(&rx, &ry);
   rvr_mouse_x_rel = (int)rx;
   rvr_mouse_y_rel = (int)ry;
   //mouse_x_rel = mouse_x_rel*pixel_scale;
   //mouse_y_rel = mouse_y_rel*pixel_scale;

   if(rvr_mouse_x>=RVR_XRES)
      rvr_mouse_x = RVR_XRES - 1;
   if(rvr_mouse_y>=RVR_YRES)
      rvr_mouse_y = RVR_YRES - 1;

   if(rvr_mouse_x<0)
      rvr_mouse_x = 0;
   if(rvr_mouse_y<0)
      rvr_mouse_y = 0;
}

void RvR_render_present()
{
   if(!SDL_RenderClear(rvr_renderer))
      RvR_log_line("SDL_RenderClear ", "%s\n", SDL_GetError());

   int width = rvr_view_width;
   int height = rvr_view_height;
   int x = rvr_view_x;
   int y = rvr_view_y;
   SDL_FRect dst_rect;
   dst_rect.x = (float)x;
   dst_rect.y = (float)y;
   dst_rect.w = (float)width;
   dst_rect.h = (float)height;

   void *data;
   int stride;

   if(!SDL_LockTexture(rvr_layer_texture, NULL, &data, &stride))
      RvR_log_line("SDL_LockTexture ", "%s\n", SDL_GetError());

   RvR_color * restrict pix = data;
   const RvR_color * restrict pal = RvR_palette();
   for(int i = 0; i<RVR_XRES * RVR_YRES; i++)
      pix[i] = pal[rvr_framebuffer[i]];

   SDL_UnlockTexture(rvr_layer_texture);

   if(!SDL_RenderTexture(rvr_renderer, rvr_layer_texture, NULL, &dst_rect))
      RvR_log_line("SDL_RenderCopy ", "%s\n", SDL_GetError());

   SDL_RenderPresent(rvr_renderer);
}

int RvR_key_down(RvR_key key)
{
   return rvr_new_key_state[key];
}

int RvR_key_pressed(RvR_key key)
{
   return rvr_new_key_state[key]&&!rvr_old_key_state[key];
}

int RvR_key_released(RvR_key key)
{
   return !rvr_new_key_state[key]&&rvr_old_key_state[key];
}

int RvR_mouse_wheel_scroll()
{
   return rvr_mouse_wheel;
}

int RvR_gamepad_down(int index, RvR_gamepad_button button)
{
   return rvr_gamepads[index].new_button_state[button];
}

int RvR_gamepad_pressed(int index, RvR_gamepad_button button)
{
   return rvr_gamepads[index].new_button_state[button]&&!rvr_gamepads[index].old_button_state[button];
}

int RvR_gamepad_released(int index, RvR_gamepad_button button)
{
   return !rvr_gamepads[index].new_button_state[button]&&rvr_gamepads[index].old_button_state[button];
}

void RvR_mouse_relative_pos(int *x, int *y)
{
   *x = rvr_mouse_x_rel;
   *y = rvr_mouse_y_rel;
}

void RvR_mouse_pos(int *x, int *y)
{
   *x = rvr_mouse_x;
   *y = rvr_mouse_y;
}

void RvR_mouse_set_pos(int x, int y)
{
   rvr_mouse_x = (int)((float)x * rvr_pixel_scale);
   rvr_mouse_y = (int)((float)y * rvr_pixel_scale);

   SDL_WarpMouseInWindow(rvr_sdl_window, (float)(rvr_mouse_x + rvr_view_x), (float)(rvr_mouse_y + rvr_view_y));
}

void RvR_text_input_start(char *text, int max_length)
{
   rvr_text_input = text;
   rvr_text_input_active = 1;
   rvr_text_input_max = max_length;

   SDL_StartTextInput(rvr_sdl_window);
}

void RvR_text_input_end()
{
   rvr_text_input_active = 0;

   SDL_StopTextInput(rvr_sdl_window);
}

uint8_t *RvR_framebuffer()
{
   return rvr_framebuffer;
}

uint32_t RvR_frame()
{
   return rvr_core_frame;
}

int RvR_frametime()
{
   return (int)((rvr_frametime * 10000) / SDL_GetPerformanceFrequency());
}

int RvR_frametime_average()
{
   int sum = 0;
   for(int i = 0; i<32; i++) sum += rvr_core_frametimes[i];
   return sum / 32;
}

int RvR_xres()
{
   return RVR_XRES;
}

int RvR_yres()
{
   return RVR_YRES;
}

int RvR_fps()
{
   return RVR_FPS;
}

static void rvr_update_viewport()
{
   SDL_GetWindowSize(rvr_sdl_window, &rvr_window_width, &rvr_window_height);

   float ratio = (float)rvr_window_width / (float)rvr_window_height;
   //float width_adjust = ((float)RVR_XRES/(float)RVR_YRES)*(float)window_width;
   //float height_adjust = ((float)RVR_XRES/(float)RVR_YRES)*(float)window_height;

   if(ratio>(float)RVR_XRES / (float)RVR_YRES)
   {
      rvr_view_height = rvr_window_height;
      rvr_view_width = (int)(((float)RVR_XRES / (float)RVR_YRES) * (float)rvr_window_height);
   }
   else
   {
      rvr_view_width = rvr_window_width;
      rvr_view_height = (int)(((float)RVR_YRES / (float)RVR_XRES) * (float)rvr_window_width);
   }

   rvr_view_x = (rvr_window_width - rvr_view_width) / 2;
   rvr_view_y = (rvr_window_height - rvr_view_height) / 2;

   rvr_pixel_scale = (float)rvr_view_width / (float)RVR_XRES;
}

static int rvr_get_gamepad_index(int which)
{
   for(int i = 0; i<4; i++)
      if(rvr_gamepads[i].connected&&rvr_gamepads[i].id==which)
         return i;

   return -1;
}
//-------------------------------------
