/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
typedef enum
{
   CONFIG_INT,
   CONFIG_KEY,
}Config_type;
//-------------------------------------

//Variables
RvR_key config_move_forward = RVR_KEY_W;
RvR_key config_move_backward = RVR_KEY_S;
RvR_key config_strafe_left = RVR_KEY_A;
RvR_key config_strafe_right = RVR_KEY_D;
RvR_key config_jump = RVR_KEY_SPACE;
RvR_key config_use = RVR_BUTTON_RIGHT;
RvR_key config_crouch = RVR_KEY_LCTRL;
//-------------------------------------

//Function prototypes
static char *config_ini_parse(const char *path);
static char *config_ini(const char *s);
static void ini_read(char *config, void *dst, Config_type type, const char *ident);
static RvR_key config_strtokey(const char *ident);
static const char *config_keytostr(RvR_key key);
//-------------------------------------

//Function implementations

void config_read(const char *path)
{
   char *ini = config_ini_parse(path);
   if(ini==NULL)
   {
      config_write(path);
      return;
   }

   ini_read(ini, &config_move_forward, CONFIG_KEY, "forward");
   ini_read(ini, &config_move_backward, CONFIG_KEY, "backward");
   ini_read(ini, &config_strafe_left, CONFIG_KEY, "strafe_left");
   ini_read(ini, &config_strafe_right, CONFIG_KEY, "strafe_right");
   ini_read(ini, &config_jump, CONFIG_KEY, "jump");
   ini_read(ini, &config_use, CONFIG_KEY, "use");
   ini_read(ini, &config_crouch, CONFIG_KEY, "crouch");

   RvR_free(ini);
}

void config_write(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw, path, "w");
   if(!RvR_rw_valid(&rw))
      return;

   RvR_rw_printf(&rw, ";Action\n");
   RvR_rw_printf(&rw, "forward=%s\n", config_keytostr(config_move_forward));
   RvR_rw_printf(&rw, "backward=%s\n", config_keytostr(config_move_backward));
   RvR_rw_printf(&rw, "strafe_left=%s\n", config_keytostr(config_strafe_left));
   RvR_rw_printf(&rw, "strafe_right=%s\n", config_keytostr(config_strafe_right));
   RvR_rw_printf(&rw, "jump=%s\n", config_keytostr(config_jump));
   RvR_rw_printf(&rw, "use=%s\n", config_keytostr(config_use));
   RvR_rw_printf(&rw, "crouch=%s\n", config_keytostr(config_crouch));

   RvR_rw_close(&rw);
}

static char *config_ini_parse(const char *path)
{
   RvR_rw rw = {0};
   RvR_rw_init_path(&rw, path, "r");

   if(!RvR_rw_valid(&rw))
   {
      return NULL;
   }

   RvR_rw_seek(&rw, 0, SEEK_END);
   long size = RvR_rw_tell(&rw);
   RvR_rw_seek(&rw, 0, SEEK_SET);
   char *buffer_in = RvR_malloc(sizeof(*buffer_in) * (size + 1), "config_read file string");
   RvR_rw_read(&rw, buffer_in, sizeof(*buffer_in) * size, 1);
   buffer_in[size] = '\0';
   RvR_rw_close(&rw);

   char *kv = config_ini(buffer_in);
   RvR_free(buffer_in);

   return kv;
}

static void ini_read(char *config, void *dst, Config_type type, const char *ident)
{
   RvR_error_check(config!=NULL, "ini_read", "argument 'config' must be non-NULL\n");
   RvR_error_check(dst!=NULL, "ini_read", "argument 'dst' must be non-NULL\n");
   RvR_error_check(ident!=NULL, "ini_read", "argument 'ident' must be non-NULL\n");

   char *iter = NULL;
   for(iter = config; iter[0];)
   {
      if(strcmp(ident, iter)==0)
      {
         while(*iter++);

         switch(type)
         {
         case CONFIG_INT: *((int *)dst) = atoi(iter); break;
         case CONFIG_KEY: *((RvR_key *)dst) = config_strtokey(iter); break;
         }

         return;
      }

      while(*iter++);
      while(*iter++);
   }

   RvR_error_fail("ini_read", "identifier '%s' not found in ini\n", ident);

RvR_err:
   return;
}

static RvR_key config_strtokey(const char *ident)
{
   struct
   {
      const char *ident;
      RvR_key key;
   } keys[] =
   {
      {"a", RVR_KEY_A},
      {"b", RVR_KEY_B},
      {"c", RVR_KEY_C},
      {"d", RVR_KEY_D},
      {"e", RVR_KEY_E},
      {"f", RVR_KEY_F},
      {"g", RVR_KEY_G},
      {"h", RVR_KEY_H},
      {"i", RVR_KEY_I},
      {"j", RVR_KEY_J},
      {"k", RVR_KEY_K},
      {"l", RVR_KEY_L},
      {"m", RVR_KEY_M},
      {"n", RVR_KEY_N},
      {"o", RVR_KEY_O},
      {"p", RVR_KEY_P},
      {"q", RVR_KEY_Q},
      {"r", RVR_KEY_R},
      {"s", RVR_KEY_S},
      {"t", RVR_KEY_T},
      {"u", RVR_KEY_U},
      {"v", RVR_KEY_V},
      {"w", RVR_KEY_W},
      {"x", RVR_KEY_X},
      {"y", RVR_KEY_Y},
      {"z", RVR_KEY_Z},

      {"f1", RVR_KEY_F1},
      {"f2", RVR_KEY_F2},
      {"f3", RVR_KEY_F3},
      {"f4", RVR_KEY_F4},
      {"f5", RVR_KEY_F5},
      {"f6", RVR_KEY_F6},
      {"f7", RVR_KEY_F7},
      {"f8", RVR_KEY_F8},
      {"f9", RVR_KEY_F9},
      {"f10", RVR_KEY_F10},
      {"f11", RVR_KEY_F11},
      {"f12", RVR_KEY_F12},

      {"0", RVR_KEY_0},
      {"1", RVR_KEY_1},
      {"2", RVR_KEY_2},
      {"3", RVR_KEY_3},
      {"4", RVR_KEY_4},
      {"5", RVR_KEY_5},
      {"6", RVR_KEY_6},
      {"7", RVR_KEY_7},
      {"8", RVR_KEY_8},
      {"9", RVR_KEY_9},

      {"up", RVR_KEY_UP},
      {"down", RVR_KEY_DOWN},
      {"left", RVR_KEY_LEFT},
      {"right", RVR_KEY_RIGHT},

      {"comma", RVR_KEY_COMMA},
      {"period", RVR_KEY_PERIOD},
      {"space", RVR_KEY_SPACE},
      {"tab", RVR_KEY_TAB},
      {"lshift", RVR_KEY_LSHIFT},
      {"rshift", RVR_KEY_RSHIFT},
      {"lctrl", RVR_KEY_LCTRL},
      {"rctrl", RVR_KEY_RCTRL},
      {"lalt", RVR_KEY_LALT},
      {"ralt", RVR_KEY_RALT},
      {"insert", RVR_KEY_INS},
      {"delete", RVR_KEY_DEL},
      {"home", RVR_KEY_HOME},
      {"end", RVR_KEY_END},
      {"pgup", RVR_KEY_PGUP},
      {"pgdn", RVR_KEY_PGDN},
      {"back", RVR_KEY_BACK},
      {"escape", RVR_KEY_ESCAPE},
      {"return", RVR_KEY_RETURN},
      {"enter", RVR_KEY_ENTER},
      {"pause", RVR_KEY_PAUSE},
      {"scroll", RVR_KEY_SCROLL},
      {"np0", RVR_KEY_NP0},
      {"np1", RVR_KEY_NP1},
      {"np2", RVR_KEY_NP2},
      {"np3", RVR_KEY_NP3},
      {"np4", RVR_KEY_NP4},
      {"np5", RVR_KEY_NP5},
      {"np6", RVR_KEY_NP6},
      {"np7", RVR_KEY_NP7},
      {"np8", RVR_KEY_NP8},
      {"np9", RVR_KEY_NP9},
      {"npmul", RVR_KEY_NP_MUL},
      {"npdiv", RVR_KEY_NP_DIV},
      {"npadd", RVR_KEY_NP_ADD},
      {"npsub", RVR_KEY_NP_SUB},
      {"npdec", RVR_KEY_NP_DECIMAL},
      {"npenter", RVR_KEY_NP_ENTER},
      {"mleft", RVR_BUTTON_LEFT},
      {"mright", RVR_BUTTON_RIGHT},
      {"mmiddle", RVR_BUTTON_MIDDLE},
      {"mx1", RVR_BUTTON_X1},
      {"mx2", RVR_BUTTON_X2},
   };

   for(int i = 0; i<sizeof(keys) / sizeof(keys[0]); i++)
   {
      if(strcmp(ident, keys[i].ident)==0)
         return keys[i].key;
   }

   return RVR_KEY_NONE;
}

static const char *config_keytostr(RvR_key key)
{
   switch(key)
   {
   case RVR_KEY_A: return "a";
   case RVR_KEY_B: return "b";
   case RVR_KEY_C: return "c";
   case RVR_KEY_D: return "d";
   case RVR_KEY_E: return "e";
   case RVR_KEY_F: return "f";
   case RVR_KEY_G: return "g";
   case RVR_KEY_H: return "h";
   case RVR_KEY_I: return "i";
   case RVR_KEY_J: return "j";
   case RVR_KEY_K: return "k";
   case RVR_KEY_L: return "l";
   case RVR_KEY_M: return "m";
   case RVR_KEY_N: return "n";
   case RVR_KEY_O: return "o";
   case RVR_KEY_P: return "p";
   case RVR_KEY_Q: return "q";
   case RVR_KEY_R: return "r";
   case RVR_KEY_S: return "s";
   case RVR_KEY_T: return "t";
   case RVR_KEY_U: return "u";
   case RVR_KEY_V: return "v";
   case RVR_KEY_W: return "w";
   case RVR_KEY_X: return "x";
   case RVR_KEY_Y: return "y";
   case RVR_KEY_Z: return "z";
   case RVR_KEY_F1: return "f1";
   case RVR_KEY_F2: return "f2";
   case RVR_KEY_F3: return "f3";
   case RVR_KEY_F4: return "f4";
   case RVR_KEY_F5: return "f5";
   case RVR_KEY_F6: return "f6";
   case RVR_KEY_F7: return "f7";
   case RVR_KEY_F8: return "f8";
   case RVR_KEY_F9: return "f9";
   case RVR_KEY_F10: return "f10";
   case RVR_KEY_F11: return "f11";
   case RVR_KEY_F12: return "f12";
   case RVR_KEY_0: return "0";
   case RVR_KEY_1: return "1";
   case RVR_KEY_2: return "2";
   case RVR_KEY_3: return "3";
   case RVR_KEY_4: return "4";
   case RVR_KEY_5: return "5";
   case RVR_KEY_6: return "6";
   case RVR_KEY_7: return "7";
   case RVR_KEY_8: return "8";
   case RVR_KEY_9: return "9";
   case RVR_KEY_UP: return "up";
   case RVR_KEY_DOWN: return "down";
   case RVR_KEY_LEFT: return "left";
   case RVR_KEY_RIGHT: return "right";
   case RVR_KEY_COMMA: return "comma";
   case RVR_KEY_PERIOD: return "period";
   case RVR_KEY_SPACE: return "space";
   case RVR_KEY_TAB: return "tab";
   case RVR_KEY_LSHIFT: return "lshift";
   case RVR_KEY_RSHIFT: return "rshift";
   case RVR_KEY_LCTRL: return "lctrl";
   case RVR_KEY_RCTRL: return "rctrl";
   case RVR_KEY_LALT: return "lalt";
   case RVR_KEY_RALT: return "ralt";
   case RVR_KEY_INS: return "insert";
   case RVR_KEY_DEL: return "delete";
   case RVR_KEY_HOME: return "home";
   case RVR_KEY_END: return "end";
   case RVR_KEY_PGUP: return "pgup";
   case RVR_KEY_PGDN: return "pgdn";
   case RVR_KEY_BACK: return "back";
   case RVR_KEY_ESCAPE: return "escape";
   case RVR_KEY_RETURN: return "return";
   case RVR_KEY_ENTER: return "enter";
   case RVR_KEY_PAUSE: return "pause";
   case RVR_KEY_SCROLL: return "scroll";
   case RVR_KEY_NP0: return "np0";
   case RVR_KEY_NP1: return "np1";
   case RVR_KEY_NP2: return "np2";
   case RVR_KEY_NP3: return "np3";
   case RVR_KEY_NP4: return "np4";
   case RVR_KEY_NP5: return "np5";
   case RVR_KEY_NP6: return "np6";
   case RVR_KEY_NP7: return "np7";
   case RVR_KEY_NP8: return "np8";
   case RVR_KEY_NP9: return "np9";
   case RVR_KEY_NP_MUL: return "npmul";
   case RVR_KEY_NP_DIV: return "npdiv";
   case RVR_KEY_NP_ADD: return "npadd";
   case RVR_KEY_NP_SUB: return "npsub";
   case RVR_KEY_NP_DECIMAL: return "npdec";
   case RVR_KEY_NP_ENTER: return "npenter";
   case RVR_BUTTON_LEFT: return "mleft";
   case RVR_BUTTON_RIGHT: return "mright";
   case RVR_BUTTON_MIDDLE: return "mmiddle";
   case RVR_BUTTON_X1: return "mx1";
   case RVR_BUTTON_X2: return "mx2";

   default: return "unknown";
   }
}

//Ini parser by r-lyeh: https://github.com/r-lyeh/tinybits
//Original header:
// ini+, extended ini format
// - rlyeh, public domain
//
// # spec
//
//   ; line comment
//   [details]          ; map section name (optional)
//   user=john          ; key and value (mapped here as details.user=john)
//   +surname=doe jr.   ; sub-key and value (mapped here as details.user.surname=doe jr.)
//   color=240          ; key and value |
//   color=253          ; key and value |> array: color[0], color[1] and color[2]
//   color=255          ; key and value |
//   color=             ; remove key/value(s)
//   color=white        ; recreate key; color[1] and color[2] no longer exist
//   []                 ; unmap section
//   -note=keys may start with symbols (except plus and semicolon)
//   -note=linefeeds are either \r, \n or \r\n.
//   -note=utf8 everywhere.
//
static char *config_ini(const char *s)
{
   char *map = NULL;
   int mapcap = 0;
   int maplen = 0;
   enum
   {
      DEL, REM, TAG, KEY, SUB, VAL
   } fsm = DEL;
   const char *cut[6] = {0};
   const char *end[6] = {0};

   while(*s)
   {
      while(*s&&(*s==' '||*s =='\t'||*s=='\r'||*s=='\n')) ++s;

      if(*s ==';') cut[fsm = REM] = ++s;
      else if(*s=='[') cut[fsm = TAG] = ++s;
      else if(*s=='+') cut[fsm = SUB] = ++s;
      else if(*s=='=') cut[fsm = VAL] = ++s;
      else if(*s>' '&&*s<='z'&&*s!=']') cut[fsm = KEY] = cut[SUB] = end[SUB] = s;
      else { ++s; continue; }

      if(fsm==REM) { while(*s&&*s!='\r'&&*s!='\n') ++s; }
      else if(fsm==TAG) { while(*s&&*s!='\r'&&*s!='\n'&&*s!=']') ++s; end[fsm] = s; }
      else if(fsm==KEY) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==SUB) { while(*s&&*s> ' '&&*s<='z'&&*s!='=') ++s; end[fsm] = s; }
      else if(fsm==VAL)
      {
         char buf[256] = {0};
         char *key = NULL;
         int reqlen = 0;

         while(*s && *s >= ' ' && *s <= 'z' && *s != ';') ++s;
         end[fsm] = s;
         while(end[fsm][-1]==' ') --end[fsm];
         key = buf;
         if(end[TAG] - cut[TAG])key += sprintf(key, "%.*s.", (int)(end[TAG] - cut[TAG]), cut[TAG]);
         if(end[KEY] - cut[KEY])key += sprintf(key, "%.*s", (int)(end[KEY] - cut[KEY]), cut[KEY]);
         if(end[SUB] - cut[SUB])key += sprintf(key, ".%.*s", (int)(end[SUB] - cut[SUB]), cut[SUB]);
         reqlen = (key - buf) + 1 + (end[VAL] - cut[VAL]) + 1 + 1;
         if((reqlen + maplen)>=mapcap) { map = RvR_realloc(map, (mapcap += reqlen + 512), "config_ini map"); }
         sprintf(map + maplen, "%.*s%c%.*s%c%c", (int)(key - buf), buf, 0, (int)(end[VAL] - cut[VAL]), cut[VAL], 0, 0);
         maplen += reqlen - 1;
      }
   }

   return map;
}
//-------------------------------------
