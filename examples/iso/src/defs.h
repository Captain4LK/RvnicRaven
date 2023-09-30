/*
RvnicRaven - iso roguelike

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _DEFS_H_

#define _DEFS_H_

#include "material.h"

//DO NOT LOAD ANY DEFS WHILE A GAME IS ACTIVE
//I.E: ONLY LOAD THEM AT STARTUP
void defs_init(void);
void defs_load(const char *path);

const MaterialDef *defs_get_material(const char *name);

#endif
