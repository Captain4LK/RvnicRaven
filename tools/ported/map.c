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
#include <string.h>
#include <time.h>
#include <inttypes.h>

#include "cute_path.h"
#include "cute_files.h"

#include "RvR/RvR.h"
#include "RvR/RvR_portal.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "map.h"
#include "editor.h"
#include "undo.h"
#include "texture.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
RvR_port_map *map = NULL;
Map_sprite *map_sprites = NULL;

/*static struct
{
   char (*data)[128];
   unsigned data_used;
   unsigned data_size;
} path_list = {0};*/
//static Map_list map_list = {0};
static char map_path[128];

static Map_sprite *map_sprite_pool = NULL;
//-------------------------------------

//Function prototypes
static void map_list_add(const char *path);
//-------------------------------------

//Function implementations

void map_load(const char *path)
{
   //Free old sprites
   while(map_sprites!=NULL)
      map_sprite_free(map_sprites);

   map_set_path(path);
   map = RvR_port_map_load_path(map_path);

   /*//Load sprites
   printf("%d Sprites\n", map->sprite_count);
   for(int i = 0; i<map->sprite_count; i++)
   {
      RvR_ray_map_sprite *s = map->sprites + i;
      Map_sprite *ms = map_sprite_new();

      ms->texture = s->texture;
      ms->extra0 = s->extra0;
      ms->extra1 = s->extra1;
      ms->extra2 = s->extra2;
      ms->flags = s->flags;
      ms->direction = s->direction;
      ms->x = s->x;
      ms->y = s->y;
      ms->z = s->z;
      //ms->pos = s->pos;

      map_sprite_add(ms);
   }
*/
   camera.x = 0;
   camera.y = 0;
   camera.z = 1024;

   undo_reset();
}

void map_new()
{
   //Free old sprites
   while(map_sprites!=NULL)
      map_sprite_free(map_sprites);

   map_path[0] = '\0';
   //snprintf(map_path, 127, "map%" PRIu64 ".map", (uint64_t)time(NULL));
   map = RvR_port_map_create();
   //printf("%d\n",texture_sky);

   camera.x = 0;
   camera.y = 0;
   camera.z = 1024;

   map->sprite_count = 1;
   map->sprites = RvR_malloc(sizeof(*map->sprites) * map->sprite_count, "Map sprites");
   memset(map->sprites, 0, sizeof(*map->sprites) * map->sprite_count);
   map->sprites[0].x = 0;
   map->sprites[0].y = 0;
   map->sprites[0].z = 0;
   map->sprites[0].x_units = 16;
   map->sprites[0].y_units = 16;
   map->sprites[0].sector = 0;
   map->sprites[0].tex = 0;

   undo_reset();

   /*int16_t sector = RvR_port_sector_new(map, 0, 0);
   map->sectors[sector].floor = 0;
   map->sectors[sector].ceiling = 2 * 1024;
   map->sectors[sector].floor_tex = 15;
   map->sectors[sector].ceiling_tex = 15;
   int16_t w = RvR_port_wall_append(map, sector, 8 * 1024, 0);
   RvR_port_wall_append(map, sector, 8 * 1024, 8 * 1024);
   RvR_port_wall_append(map, sector, 0, 8 * 1024);
   RvR_port_wall_append(map, sector, 0, 0);
   RvR_port_wall_insert(map, w, 12 * 1024, 4 * 1024);

   RvR_port_wall_append(map, sector, 3 * 1024, 3 * 1024);
   RvR_port_wall_append(map, sector, 5 * 1024, 3 * 1024);
   RvR_port_wall_append(map, sector, 5 * 1024, 5 * 1024);
   RvR_port_wall_append(map, sector, 3 * 1024, 5 * 1024);
   RvR_port_wall_append(map, sector, 2 * 1024, 4 * 1024);
   w = RvR_port_wall_append(map, sector, 3 * 1024, 3 * 1024);
   int16_t s = RvR_port_sector_make_inner(map, w);
   map->sectors[s].floor = 512;
   map->sectors[s].ceiling  = 2 * 1024 - 512;
   map->sectors[s].floor_tex = 5;
   map->sectors[s].ceiling_tex = 5;*/

   //RvR_port_wall_insert(&map, map->sectors[s].wall_first, 8 * 1024, 4 * 1024);
}

void map_save()
{
   //Save sprites
   //count
   int sprite_count = 0;
   Map_sprite *s = map_sprites;
   while(s!=NULL)
   {
      s = s->next;
      sprite_count++;
   }
   /*map->sprite_count = sprite_count;
   map->sprites = RvR_realloc(map->sprites, sizeof(*map->sprites) * map->sprite_count, "RvR_ray map sprites grow");
   s = map_sprites;
   for(int i = 0; s!=NULL; i++)
   {
      map->sprites[i].texture = s->texture;
      map->sprites[i].x = s->x;
      map->sprites[i].y = s->y;
      map->sprites[i].z = s->z;
      map->sprites[i].direction = s->direction;
      map->sprites[i].extra0 = s->extra0;
      map->sprites[i].extra1 = s->extra1;
      map->sprites[i].extra2 = s->extra2;
      map->sprites[i].flags = s->flags;
      s = s->next;
   }*/

   puts(map_path);
   RvR_port_map_save(map, map_path);
}

void map_set_path(const char *path)
{
   strncpy(map_path, path, 128);
   map_path[127] = '\0';
}

const char *map_path_get()
{
   return map_path;
}

/*void map_path_add(const char *path)
{
   if(path_list.data==NULL)
   {
      path_list.data_used = 0;
      path_list.data_size = 1;
      path_list.data = RvR_malloc(sizeof(*path_list.data) * path_list.data_size, "rayed map path list");
   }

   for(unsigned i = 0; i<path_list.data_used; i++)
      if(strcmp(path, path_list.data[i])==0)
         return;

   strncpy(path_list.data[path_list.data_used], path, 127);
   path_list.data_used++;

   if(path_list.data_used>=path_list.data_size)
   {
      path_list.data_size += 1;
      path_list.data = RvR_realloc(path_list.data, sizeof(*path_list.data) * path_list.data_size, "rayed map path list grow");
   }
}

static void map_list_add(const char *path)
{
   if(map_list.data==NULL)
   {
      map_list.data_used = 0;
      map_list.data_size = 1;
      map_list.data = RvR_malloc(sizeof(*map_list.data) * map_list.data_size, "rayed map list");
   }

   for(unsigned i = 0; i<map_list.data_used; i++)
      if(strcmp(path, map_list.data[i])==0)
         return;

   strncpy(map_list.data[map_list.data_used], path, 127);
   map_list.data_used++;

   if(map_list.data_used>=map_list.data_size)
   {
      map_list.data_size += 1;
      map_list.data = RvR_realloc(map_list.data, sizeof(*map_list.data) * map_list.data_size, "rayed map list grow");
   }
}*/

/*Map_list *map_list_get()
{
   char path[128];
   map_list.data_used = 0;

   for(unsigned i = 0; i<path_list.data_used; i++)
   {
      cf_dir_t dir = {0};
      //RvR_log_line("map_list_get ","opening directory %s\n",path_list.data[i]);
      cf_dir_open(&dir, path_list.data[i]);

      while(dir.has_next)
      {
         cf_file_t file;
         cf_read_file(&dir, &file);
         //RvR_log_line("map_list_get ","found file %s\n",file.path);
         if(cf_match_ext(&file, ".map"))
         {
            snprintf(path, 128, "%s%s", path_list.data[i], file.name);
            map_list_add(path);
         }

         cf_dir_next(&dir);
      }

      cf_dir_close(&dir);
   }

   return &map_list;
}*/

Map_sprite *map_sprite_new()
{
   if(map_sprite_pool==NULL)
   {
      Map_sprite *ms = RvR_malloc(sizeof(*ms) * 256, "rayed map sprite pool");
      memset(ms, 0, sizeof(*ms) * 256);

      for(int i = 0; i<255; i++)
         ms[i].next = &ms[i + 1];
      map_sprite_pool = &ms[0];
   }

   Map_sprite *ms = map_sprite_pool;
   map_sprite_pool = ms->next;
   ms->next = NULL;
   ms->prev_next = NULL;

   return ms;
}

void map_sprite_add(Map_sprite *sp)
{
   sp->prev_next = &map_sprites;
   if(map_sprites!=NULL)
      map_sprites->prev_next = &sp->next;

   sp->next = map_sprites;
   map_sprites = sp;
}

void map_sprite_free(Map_sprite *sp)
{
   if(sp==NULL)
      return;

   *sp->prev_next = sp->next;
   if(sp->next!=NULL)
      sp->next->prev_next = sp->prev_next;

   sp->next = map_sprite_pool;
   map_sprite_pool = sp;
}
//-------------------------------------
