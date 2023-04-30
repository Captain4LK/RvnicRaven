/*
RvnicRaven - pak ressource managment

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PAK_H_

#define _RVR_PAK_H_

void        RvR_pak_add(const char *path);
void        RvR_pak_create_from_csv(const char *path_csv, const char *path_pak);
void        RvR_pak_flush();
void        RvR_lump_add(const char *name, const char *path);
void       *RvR_lump_get(const char *name, uint32_t *size);
const char *RvR_lump_get_path(const char *name);
int         RvR_lump_exists(const char *name);

#endif
