/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <SDL2/SDL.h>

#define CUTE_PATH_IMPLEMENTATION
#include "cute_path.h"

#define CUTE_FILES_IMPLEMENTATION
#include "cute_files.h"

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "color.h"
#include "texture.h"
#include "map.h"
#include "editor.h"
#include "util.h"

#include "../../libraries/HLH_gui/HLH_gui.h"
#include "../../libraries/HLH_gui/HLH_gui_all.c"
#include "RvR/RvR_gui.h"
//-------------------------------------

//#defines
#define MEM_SIZE (1 << 24)
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static uint8_t mem[MEM_SIZE];
static HLH_gui_window *window_root;
//-------------------------------------

//Function prototypes
static void main_loop();

static void ui_construct();
static void ui_construct_ask_new();
static int menu_file_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp);
static int menu_edit_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp);
static int menu_help_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp);
static int ask_new_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp);
//-------------------------------------

//Function implementations

int main(int argc, char **argv)
{
   HLH_gui_init();

   if(argc<2)
   {
      puts("No pak path specified!");
      return -1;
   }

   //Init memory manager
   RvR_malloc_init(mem, MEM_SIZE);

   ui_construct();

   //Init RvnicRaven core
   RvR_init("Ported", 0);
   RvR_mouse_relative(0);
   //RvR_mouse_show(0);
   RvR_key_repeat(1);

   for(int i = 1; i<argc; i++)
      RvR_pak_add(argv[i]);
   RvR_palette_load(0);
   RvR_render_font_set(0xF000);

   colors_find();
   texture_list_create();
   map_new();
   texture_list_used_create();

   char path[512];
   path_pop(argv[0], path, NULL);
   strcat(path, "/");
   map_path_add(path);
   path_pop(argv[1], path, NULL);
   strcat(path, "/");
   map_path_add(path);

   editor_init();

   /*while(RvR_running())
   {
      RvR_update();

      editor_update();
      editor_draw();

      if(RvR_key_pressed(RVR_KEY_M))
         RvR_malloc_report();

      RvR_render_present();
   }

   map_set_path("autosave.map");
   map_save();*/

   return HLH_gui_message_loop();
}

static void main_loop()
{
   RvR_update();

   editor_update();
   editor_draw();

   if(RvR_key_pressed(RVR_KEY_M))
      RvR_malloc_report();

   RvR_render_present();
}

static void ui_construct()
{
   HLH_gui_window *win = HLH_gui_window_create("Ported",800,600,NULL);
   window_root = win;

   const char *menu0[] = 
   {
      "New",
      "Save",
      "Save As",
      "Load",
   };
   const char *menu1[] = 
   {
      "Test 1",
      "Test 2",
      "Test 3",
   };
   const char *menu2[] = 
   {
      "About",
      "Test 2",
      "Test 3",
   };

   HLH_gui_element *menus[3];
   menus[0] = (HLH_gui_element *)HLH_gui_menu_create(&win->e,HLH_GUI_STYLE_01|HLH_GUI_NO_PARENT|HLH_GUI_OVERLAY,HLH_GUI_FILL_X|HLH_GUI_STYLE_01,menu0,4,menu_file_msg);
   menus[1] = (HLH_gui_element *)HLH_gui_menu_create(&win->e,HLH_GUI_STYLE_01|HLH_GUI_NO_PARENT|HLH_GUI_OVERLAY,HLH_GUI_FILL_X|HLH_GUI_STYLE_01,menu1,3,menu_edit_msg);
   menus[2] = (HLH_gui_element *)HLH_gui_menu_create(&win->e,HLH_GUI_STYLE_01|HLH_GUI_NO_PARENT|HLH_GUI_OVERLAY,HLH_GUI_FILL_X|HLH_GUI_STYLE_01,menu2,3,menu_help_msg);

   const char *menubar[] = 
   {
      "File",
      "Edit",
      "Help",
   };

   HLH_gui_group *group_root = HLH_gui_group_create(&win->e,HLH_GUI_EXPAND);
   HLH_gui_menubar_create(&group_root->e,HLH_GUI_FILL_X,HLH_GUI_PACK_WEST|HLH_GUI_STYLE_01,menubar,menus,3,NULL);
   HLH_gui_separator_create(&group_root->e,HLH_GUI_FILL_X,0);

   HLH_gui_group *group = HLH_gui_group_create(&group_root->e,HLH_GUI_EXPAND);
   HLH_gui_rvr *rvr = HLH_gui_rvr_create(&group->e,HLH_GUI_EXPAND,main_loop);
}

static int menu_file_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp)
{
   HLH_gui_menubutton *m = (HLH_gui_menubutton *)e;
   if(msg==HLH_GUI_MSG_CLICK_MENU)
   {
      //New
      if(m->index==0)
      {
         //user if sure
         ui_construct_ask_new();
      }
      //Save
      else if(m->index==1)
      {
      }
      //Save as
      else if(m->index==2)
      {
      }
      //Load
      else if(m->index==3)
      {
      }
   }
   return 0;
}

static int menu_edit_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp)
{
   HLH_gui_menubutton *m = (HLH_gui_menubutton *)e;
   if(msg==HLH_GUI_MSG_CLICK_MENU)
   {
   }
   return 0;
}

static int menu_help_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp)
{
   HLH_gui_menubutton *m = (HLH_gui_menubutton *)e;
   if(msg==HLH_GUI_MSG_CLICK_MENU)
   {
   }
   return 0;
}

static void ui_construct_ask_new()
{
   HLH_gui_window *win = HLH_gui_window_create("Create new map",300,100,NULL);
   HLH_gui_window_block(window_root,win);
   HLH_gui_group *group = HLH_gui_group_create(&win->e,HLH_GUI_EXPAND);

   HLH_gui_label_create(&group->e,0,"Are you sure you want");
   HLH_gui_label_create(&group->e,0,"to start a new map?");
   group = HLH_gui_group_create(&group->e,HLH_GUI_PACK_SOUTH);
   HLH_gui_button *button = HLH_gui_button_create(&group->e,HLH_GUI_PACK_WEST|HLH_GUI_MAX_X,"Cancel",NULL);
   button->e.msg_usr = ask_new_msg;
   button->e.usr = 0;
   button = HLH_gui_button_create(&group->e,HLH_GUI_PACK_WEST|HLH_GUI_MAX_X,"Save",NULL);
   button->e.msg_usr = ask_new_msg;
   button->e.usr = 1;
   button = HLH_gui_button_create(&group->e,HLH_GUI_PACK_WEST|HLH_GUI_MAX_X,"Confirm",NULL);
   button->e.msg_usr = ask_new_msg;
   button->e.usr = 2;
}

static int ask_new_msg(HLH_gui_element *e, HLH_gui_msg msg, int di, void *dp)
{
   if(msg==HLH_GUI_MSG_CLICK)
   {
      if(e->usr==0)
      {
         HLH_gui_window_close(e->window);
      }
      else if(e->usr==1)
      {
         HLH_gui_window_close(e->window);

         //Save

         //Create New
      }
      else if(e->usr==2)
      {
         HLH_gui_window_close(e->window);

         //Create New
      }
   }

   return 0;
}
//-------------------------------------
