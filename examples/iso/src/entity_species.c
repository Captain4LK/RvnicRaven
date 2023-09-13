/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

//External includes
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <inttypes.h>
#include "RvR/RvR.h"
//-------------------------------------

//Internal includes
#include "config.h"
#include "entity.h"
#include "entity_species.h"
//-------------------------------------

//#defines
//-------------------------------------

//Typedefs
//-------------------------------------

//Variables
static Entity_species **entity_species = NULL;
//-------------------------------------

//Function prototypes
static void entity_species_load(uint16_t id);
//-------------------------------------

//Function implementations

Entity_species *entity_species_get(uint16_t id)
{
   RvR_error_check(id<ENTITY_SPECIES_MAX, "entity_species_get", "entity species id %" PRIu16 " out of bounds, max id: %d\n", id, ENTITY_SPECIES_MAX);

   if(entity_species==NULL||entity_species[id]==NULL)
      entity_species_load(id);

   return entity_species[id];

RvR_err:
   return entity_species_get(0);
}

static void entity_species_load(uint16_t id)
{
   uint8_t *mem_pak = NULL;
   unsigned size_in = 0;
   RvR_rw rw = {0};

   if(entity_species==NULL)
   {
      entity_species = RvR_malloc(sizeof(*entity_species) * ENTITY_SPECIES_MAX, "entity_species ptr array");
      memset(entity_species, 0, sizeof(*entity_species) * ENTITY_SPECIES_MAX);
   }

   //Format lump name
   char tmp[64];
   sprintf(tmp, "SPC%05d", id);

   mem_pak = RvR_lump_get(tmp, &size_in);
   RvR_error_check(size_in!=0, "entity_species_get", "SPC%05d not found\n", id);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_STATIC);
   RvR_rw_init_const_mem(&rw, mem_pak, size_in);

   entity_species[id] = RvR_malloc(sizeof(*entity_species[id]), "Entity_species");
   memset(entity_species[id], 0, sizeof(*entity_species[id]));
   entity_species[id]->flags = RvR_rw_read_u64(&rw);

   RvR_rw_close(&rw);
   RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   return;

RvR_err:

   if(mem_pak!=NULL)
      RvR_mem_tag_set(mem_pak, RVR_MALLOC_CACHE);

   if(RvR_rw_valid(&rw))
      RvR_rw_close(&rw);

   entity_species[id] = RvR_malloc(sizeof(*entity_species[id]), "Entity_species");
   memset(entity_species[id], 0, sizeof(*entity_species[id]));
}
//-------------------------------------
