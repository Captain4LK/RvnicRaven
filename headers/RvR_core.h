#ifndef _RVR_CORE_H_

/*
   RvnicRaven - core

   Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

   To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

   You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

/*
   To create implementation (the function definitions) add
      #define RVR_CORE_IMPLEMENTATION
   before including this file in *one* C file (translation unit)
*/

#define _RVR_CORE_H_

#ifndef RVR_XRES
#define RVR_XRES 640
#endif

#ifndef RVR_YRES
#define RVR_YRES 480
#endif

#ifndef RVR_FPS
#define RVR_FPS 30
#endif

typedef enum
{
   RVR_KEY_NONE,

   RVR_KEY_A, RVR_KEY_B, RVR_KEY_C, RVR_KEY_D, RVR_KEY_E, RVR_KEY_F, RVR_KEY_G,
   RVR_KEY_H, RVR_KEY_I, RVR_KEY_J, RVR_KEY_K, RVR_KEY_L, RVR_KEY_M, RVR_KEY_N,
   RVR_KEY_O, RVR_KEY_P, RVR_KEY_Q, RVR_KEY_R, RVR_KEY_S, RVR_KEY_T, RVR_KEY_U,
   RVR_KEY_V, RVR_KEY_W, RVR_KEY_X, RVR_KEY_Y, RVR_KEY_Z,
   RVR_KEY_0, RVR_KEY_1, RVR_KEY_2, RVR_KEY_3, RVR_KEY_4, RVR_KEY_5,
   RVR_KEY_6, RVR_KEY_7, RVR_KEY_8, RVR_KEY_9, RVR_KEY_COMMA, RVR_KEY_PERIOD,
   RVR_KEY_F1, RVR_KEY_F2, RVR_KEY_F3, RVR_KEY_F4, RVR_KEY_F5, RVR_KEY_F6,
   RVR_KEY_F7, RVR_KEY_F8, RVR_KEY_F9, RVR_KEY_F10, RVR_KEY_F11, RVR_KEY_F12,
   RVR_KEY_UP, RVR_KEY_DOWN, RVR_KEY_LEFT, RVR_KEY_RIGHT,
   RVR_KEY_SPACE, RVR_KEY_TAB, RVR_KEY_LSHIFT, RVR_KEY_RSHIFT, RVR_KEY_LCTRL, RVR_KEY_RCTRL,
   RVR_KEY_LALT, RVR_KEY_RALT, RVR_KEY_INS,
   RVR_KEY_DEL, RVR_KEY_HOME, RVR_KEY_END, RVR_KEY_PGUP, RVR_KEY_PGDN,
   RVR_KEY_BACK, RVR_KEY_ESCAPE, RVR_KEY_RETURN, RVR_KEY_ENTER, RVR_KEY_PAUSE, RVR_KEY_SCROLL,
   RVR_KEY_NP0, RVR_KEY_NP1, RVR_KEY_NP2, RVR_KEY_NP3, RVR_KEY_NP4, RVR_KEY_NP5, RVR_KEY_NP6, RVR_KEY_NP7, RVR_KEY_NP8, RVR_KEY_NP9,
   RVR_KEY_NP_MUL, RVR_KEY_NP_DIV, RVR_KEY_NP_ADD, RVR_KEY_NP_SUB, RVR_KEY_NP_DECIMAL, RVR_KEY_NP_ENTER,

   RVR_BUTTON_LEFT, RVR_BUTTON_RIGHT, RVR_BUTTON_MIDDLE, RVR_BUTTON_X1, RVR_BUTTON_X2,

   RVR_KEY_MAX,
}RvR_key;

typedef enum
{
   RVR_PAD_A, RVR_PAD_B, RVR_PAD_X, RVR_PAD_Y, RVR_PAD_BACK, RVR_PAD_GUIDE,
   RVR_PAD_START, RVR_PAD_LEFTSTICK, RVR_PAD_RIGHTSTICK, RVR_PAD_LEFTSHOULDER,
   RVR_PAD_RIGHTSHOULDER, RVR_PAD_UP, RVR_PAD_DOWN, RVR_PAD_LEFT, RVR_PAD_RIGHT,
   RVR_PAD_MAX,
}RvR_gamepad_button;

void     RvR_core_init(char *title, int scale);
void     RvR_core_quit();
int      RvR_core_running();
void     RvR_core_update();
int      RvR_core_frametime();
int      RvR_core_frametime_average();
void     RvR_core_render_present();
uint8_t *RvR_core_framebuffer();
uint32_t RvR_core_frame();

int  RvR_core_key_down(RvR_key key);
int  RvR_core_key_pressed(RvR_key key);
int  RvR_core_key_released(RvR_key key);
void RvR_core_key_repeat(int repeat);
int  RvR_core_gamepad_down(int index, RvR_gamepad_button button);
int  RvR_core_gamepad_pressed(int index, RvR_gamepad_button button);
int  RvR_core_gamepad_released(int index, RvR_gamepad_button button);
void RvR_core_mouse_relative_pos(int *x, int *y);
void RvR_core_mouse_pos(int *x, int *y);
void RvR_core_mouse_set_pos(int x, int y);
void RvR_core_mouse_relative(int relative);
void RvR_core_mouse_show(int show);
int  RvR_core_mouse_wheel_scroll();
void RvR_core_text_input_start(char *text, int max_length);
void RvR_core_text_input_end();

#endif

#ifdef RVR_CORE_IMPLEMENTATION
#ifndef RVR_CORE_IMPLEMENTATION_ONCE
#define RVR_CORE_IMPLEMENTATION_ONCE

#ifndef RvR_log
#define RvR_log(...) while(0)
#endif

#ifndef RvR_error_fail
#define RvR_error_fail(w, ...) do { goto RvR_err; } while(0)
#endif

#ifndef RvR_error_check
#define RvR_error_check(X, w, ...) do { if(!(X)) RvR_error_fail(w, __VA_ARGS__); } while(0)
#endif

#ifndef RvR_log_line
#define RvR_log_line(w, ...) while(0)
#endif

#define RvR_core_min(a, b) ((a)<(b)?(a):(b))
#define RvR_core_max(a, b) ((a)>(b)?(a):(b))

typedef struct
{
   SDL_GameController *gamepad;
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
static uint8_t rvr_key_map[SDL_NUM_SCANCODES];
static uint8_t rvr_mouse_map[6];
static uint8_t rvr_gamepad_map[SDL_CONTROLLER_BUTTON_MAX];
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

void RvR_core_quit()
{
   rvr_core_running = 0;
}

void RvR_core_init(char *title, int scale)
{
   Uint32 flags =
#ifndef __EMSCRIPTEN__
      SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#else
      SDL_INIT_VIDEO | SDL_INIT_EVENTS;
#endif

   if(SDL_Init(flags)<0)
   {
      RvR_log_line("SDL_Init ", "%s\n", SDL_GetError());
      exit(-1);
   }

   if(scale==0)
   {
      SDL_Rect max_size;

      if(SDL_GetDisplayUsableBounds(0, &max_size)<0)
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

   }

   if(scale<=0)
      scale = 1;

   rvr_sdl_window = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, RVR_XRES * scale, RVR_YRES * scale, SDL_WINDOW_RESIZABLE);
   if(rvr_sdl_window==NULL)
   {
      RvR_log_line("SDL_CreateWindow ", "%s\n", SDL_GetError());
      exit(-1);
   }

   rvr_renderer = SDL_CreateRenderer(rvr_sdl_window, -1, SDL_RENDERER_ACCELERATED);
   if(rvr_renderer==NULL)
   {
      RvR_log_line("SDL_CreateRenderer ", "%s\n", SDL_GetError());
      exit(-1);
   }

   if(SDL_SetRenderDrawColor(rvr_renderer, 0, 0, 0, 0)!=0)
      RvR_log_line("SDL_SetRenderDrawColor ", "%s\n", SDL_GetError());

   rvr_layer_texture = SDL_CreateTexture(rvr_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, RVR_XRES, RVR_YRES);
   if(rvr_layer_texture==NULL)
      RvR_log_line("SDL_CreateTexture ", "%s\n", SDL_GetError());

   if(SDL_SetTextureBlendMode(rvr_layer_texture, SDL_BLENDMODE_BLEND)<0)
      RvR_log_line("SDL_SetTextureBlendMode ", "%s\n", SDL_GetError());

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

   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_A] = RVR_PAD_A;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_B] = RVR_PAD_B;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_X] = RVR_PAD_X;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_Y] = RVR_PAD_Y;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_BACK] = RVR_PAD_BACK;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_GUIDE] = RVR_PAD_GUIDE;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_START] = RVR_PAD_START;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_LEFTSTICK] = RVR_PAD_LEFTSTICK;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_RIGHTSTICK] = RVR_PAD_RIGHTSTICK;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_LEFTSHOULDER] = RVR_PAD_LEFTSHOULDER;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_RIGHTSHOULDER] = RVR_PAD_RIGHTSHOULDER;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_UP] = RVR_PAD_UP;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_DOWN] = RVR_PAD_DOWN;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_LEFT] = RVR_PAD_LEFT;
   rvr_gamepad_map[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] = RVR_PAD_RIGHT;

   //Clear key states, just in case,
   //should already be empty since known at compile time
   memset(rvr_new_key_state, 0, sizeof(rvr_new_key_state));
   memset(rvr_old_key_state, 0, sizeof(rvr_old_key_state));
   for(int i = 0; i<4; i++)
   {
      memset(rvr_gamepads[i].new_button_state, 0, sizeof(rvr_gamepads[i].new_button_state));
      memset(rvr_gamepads[i].old_button_state, 0, sizeof(rvr_gamepads[i].old_button_state));
   }

   int fps = RvR_core_max(1, RvR_core_min(1000, RVR_FPS));
   rvr_framedelay = SDL_GetPerformanceFrequency() / fps;

   rvr_framebuffer = RvR_malloc(RVR_XRES * RVR_YRES, "RvR framebuffer");
   memset(rvr_framebuffer, 0, RVR_XRES * RVR_YRES);
}

void RvR_core_mouse_relative(int relative)
{
   if(SDL_SetRelativeMouseMode(relative)<0)
      RvR_log_line("SDL_SetRelativeMouseMode ", "%s\n", SDL_GetError());
}

void RvR_core_mouse_show(int show)
{
   if(SDL_ShowCursor(show?SDL_ENABLE:SDL_DISABLE)<0)
      RvR_log_line("SDL_ShowCursor ", "%s\n", SDL_GetError());
}

void RvR_core_key_repeat(int repeat)
{
   rvr_key_repeat = repeat;
}

int RvR_core_running()
{
   return rvr_core_running;
}

void RvR_core_update()
{
   rvr_core_frame++;
   rvr_core_frametimes[rvr_core_frame & 31] = RvR_core_frametime();

   #ifndef __EMSCRIPTEN__
   rvr_frametime = SDL_GetPerformanceCounter() - rvr_framestart;

   if(rvr_framedelay>rvr_frametime)
   {
      SDL_Delay(((rvr_framedelay - rvr_frametime) * 1000) / SDL_GetPerformanceFrequency());
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
      case SDL_QUIT:
         RvR_core_quit();
         break;
      case SDL_KEYDOWN:
         if(rvr_text_input_active&&event.key.keysym.sym==SDLK_BACKSPACE&&rvr_text_input[0]!='\0')
            rvr_text_input[strlen(rvr_text_input) - 1] = '\0';
         if(event.key.state==SDL_PRESSED)
         {
            rvr_new_key_state[rvr_key_map[event.key.keysym.scancode]] = 1;
            if(event.key.repeat&&rvr_key_repeat)
               rvr_old_key_state[rvr_key_map[event.key.keysym.scancode]] = 0;
         }
         break;
      case SDL_KEYUP:
         if(event.key.state==SDL_RELEASED)
            rvr_new_key_state[rvr_key_map[event.key.keysym.scancode]] = 0;
         break;
      case SDL_MOUSEBUTTONDOWN:
         if(event.button.state==SDL_PRESSED)
            rvr_new_key_state[rvr_mouse_map[event.button.button]] = 1;
         break;
      case SDL_MOUSEBUTTONUP:
         if(event.button.state==SDL_RELEASED)
            rvr_new_key_state[rvr_mouse_map[event.button.button]] = 0;
         break;
      case SDL_TEXTINPUT:
         if(rvr_text_input_active&&strlen(rvr_text_input) + strlen(event.text.text)<rvr_text_input_max)
            strcat(rvr_text_input, event.text.text);
         break;
      case SDL_MOUSEWHEEL:
         rvr_mouse_wheel = event.wheel.y;
         break;
      case SDL_CONTROLLERBUTTONDOWN:
         if(event.cbutton.state==SDL_PRESSED)
         {
            int id = rvr_get_gamepad_index(event.cbutton.which);
            rvr_gamepads[id].new_button_state[rvr_gamepad_map[event.cbutton.button]] = 1;
         }
         break;
      case SDL_CONTROLLERBUTTONUP:
         if(event.cbutton.state==SDL_RELEASED)
         {
            int id = rvr_get_gamepad_index(event.cbutton.which);
            rvr_gamepads[id].new_button_state[rvr_gamepad_map[event.cbutton.button]] = 0;
         }
         break;
      case SDL_CONTROLLERDEVICEADDED:
      {
         int which = event.cdevice.which;
         if(which<4)
         {
            rvr_gamepads[which].gamepad = SDL_GameControllerOpen(which);
            rvr_gamepads[which].connected = 1;
            SDL_Joystick *j = SDL_GameControllerGetJoystick(rvr_gamepads[which].gamepad);
            rvr_gamepads[which].id = SDL_JoystickInstanceID(j);
         }
      }
      break;
      case SDL_CONTROLLERDEVICEREMOVED:
      {
         int which = event.cdevice.which;
         if(which<0)
            break;
         int id = rvr_get_gamepad_index(which);
         rvr_gamepads[id].connected = 0;
         SDL_GameControllerClose(rvr_gamepads[id].gamepad);
      }
      break;
      case SDL_WINDOWEVENT:
         rvr_update_viewport();
         break;
      }
   }
   //-------------------------------------------

   int x, y;
   SDL_GetMouseState(&x, &y);

   x -= rvr_view_x;
   y -= rvr_view_y;
   rvr_mouse_x = x / rvr_pixel_scale;
   rvr_mouse_y = y / rvr_pixel_scale;

   SDL_GetRelativeMouseState(&rvr_mouse_x_rel, &rvr_mouse_y_rel);
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

void RvR_core_render_present()
{
   if(SDL_RenderClear(rvr_renderer)!=0)
      RvR_log_line("SDL_RenderClear ", "%s\n", SDL_GetError());

   float width = (float)rvr_view_width;
   float height = (float)rvr_view_height;
   float x = rvr_view_x;
   float y = rvr_view_y;
   SDL_Rect dst_rect;
   dst_rect.x = x;
   dst_rect.y = y;
   dst_rect.w = width;
   dst_rect.h = height;

   void *data;
   int stride;

   if(SDL_LockTexture(rvr_layer_texture, NULL, &data, &stride)!=0)
      RvR_log_line("SDL_LockTexture ", "%s\n", SDL_GetError());

   RvR_color * restrict pix = data;
   const RvR_color * restrict pal = RvR_palette();
   for(int i = 0; i<RVR_XRES * RVR_YRES; i++)
      pix[i] = pal[rvr_framebuffer[i]];

   SDL_UnlockTexture(rvr_layer_texture);

   if(SDL_RenderCopy(rvr_renderer, rvr_layer_texture, NULL, &dst_rect)!=0)
      RvR_log_line("SDL_RenderCopy ", "%s\n", SDL_GetError());

   SDL_RenderPresent(rvr_renderer);
}

int RvR_core_key_down(RvR_key key)
{
   return rvr_new_key_state[key];
}

int RvR_core_key_pressed(RvR_key key)
{
   return rvr_new_key_state[key]&&!rvr_old_key_state[key];
}

int RvR_core_key_released(RvR_key key)
{
   return !rvr_new_key_state[key]&&rvr_old_key_state[key];
}

int RvR_core_mouse_wheel_scroll()
{
   return rvr_mouse_wheel;
}

int RvR_core_gamepad_down(int index, RvR_gamepad_button button)
{
   return rvr_gamepads[index].new_button_state[button];
}

int RvR_core_gamepad_pressed(int index, RvR_gamepad_button button)
{
   return rvr_gamepads[index].new_button_state[button]&&!rvr_gamepads[index].old_button_state[button];
}

int RvR_core_gamepad_released(int index, RvR_gamepad_button button)
{
   return !rvr_gamepads[index].new_button_state[button]&&rvr_gamepads[index].old_button_state[button];
}

void RvR_core_mouse_relative_pos(int *x, int *y)
{
   *x = rvr_mouse_x_rel;
   *y = rvr_mouse_y_rel;
}

void RvR_core_mouse_pos(int *x, int *y)
{
   *x = rvr_mouse_x;
   *y = rvr_mouse_y;
}

void RvR_core_mouse_set_pos(int x, int y)
{
   rvr_mouse_x = x;
   rvr_mouse_y = y;
   x *= rvr_pixel_scale;
   y *= rvr_pixel_scale;

   SDL_WarpMouseInWindow(rvr_sdl_window, x, y);
}

void RvR_core_text_input_start(char *text, int max_length)
{
   rvr_text_input = text;
   rvr_text_input_active = 1;
   rvr_text_input_max = max_length;

   SDL_StartTextInput();
}

void RvR_core_text_input_end()
{
   rvr_text_input_active = 0;

   SDL_StopTextInput();
}

uint8_t *RvR_core_framebuffer()
{
   return rvr_framebuffer;
}

uint32_t RvR_core_frame()
{
   return rvr_core_frame;
}

int RvR_core_frametime()
{
   return (rvr_frametime * 10000) / SDL_GetPerformanceFrequency();
}

int RvR_core_frametime_average()
{
   uint64_t sum = 0;
   for(int i = 0; i<32; i++) sum += rvr_core_frametimes[i];
   return sum / 32;
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
      rvr_view_width = ((float)RVR_XRES / (float)RVR_YRES) * (float)rvr_window_height;
   }
   else
   {
      rvr_view_width = rvr_window_width;
      rvr_view_height = ((float)RVR_YRES / (float)RVR_XRES) * (float)rvr_window_width;
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

#undef RvR_core_min
#undef RvR_core_max

#endif
#endif
