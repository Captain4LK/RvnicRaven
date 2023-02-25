/*
RvnicRaven - input and app managment

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_APP_H_

#define _RVR_APP_H_

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

void     RvR_init(char *title, int scale);
void     RvR_quit();
int      RvR_running();
void     RvR_update();
int      RvR_frametime();
int      RvR_frametime_average();
void     RvR_render_present();
uint8_t *RvR_framebuffer();
uint32_t RvR_frame();

int  RvR_key_down(RvR_key key);
int  RvR_key_pressed(RvR_key key);
int  RvR_key_released(RvR_key key);
void RvR_key_repeat(int repeat);
int  RvR_gamepad_down(int index, RvR_gamepad_button button);
int  RvR_gamepad_pressed(int index, RvR_gamepad_button button);
int  RvR_gamepad_released(int index, RvR_gamepad_button button);
void RvR_mouse_relative_pos(int *x, int *y);
void RvR_mouse_pos(int *x, int *y);
void RvR_mouse_set_pos(int x, int y);
void RvR_mouse_relative(int relative);
void RvR_mouse_show(int show);
int  RvR_mouse_wheel_scroll();
void RvR_text_input_start(char *text, int max_length);
void RvR_text_input_end();

#endif
