/*
RvnicRaven retro game engine

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _UNDO_H_

#define _UNDO_H_

void undo_init(void);
void undo_reset(void);
void undo(void);
void redo(void);

//For writing undo/redo entries
void undo_track_wall_shade(uint16_t wall);
void undo_track_floor_shade(uint16_t sector);
void undo_track_ceiling_shade(uint16_t sector);
void undo_track_sector_flag(uint16_t sector);
void undo_track_sprite_flag(uint16_t sprite);
void undo_track_sprite_dir(uint16_t sprite);
void undo_track_wall_flag(uint16_t wall);
void undo_track_wall_units(uint16_t wall);
void undo_track_sector_units(uint16_t sector);
void undo_track_sprite_units(uint16_t sprite);
void undo_track_wall_offsets(uint16_t wall);
void undo_track_sector_offsets(uint16_t sector);
void undo_track_sector_slope(uint16_t sector);
void undo_track_sector_height(uint16_t sector);
void undo_track_sprite_pos(uint16_t sprite);
void undo_track_sprite_tex(uint16_t sprite);
void undo_track_wall_tex(uint16_t wall);
void undo_track_sector_tex(uint16_t sector);
void undo_track_sprite_del(uint16_t sprite);
void undo_track_wall_move(uint16_t wall);
void undo_track_sector_add(uint16_t sector);
void undo_track_sector_add_inner(uint16_t sector);
void undo_track_sector_add_overlap(uint16_t sector);
void undo_track_sector_split(uint16_t sector);
void undo_track_sector_connect(uint16_t sector);
void undo_track_wall_insert(uint16_t wall);
void undo_track_sector_delete(uint16_t sector);
void undo_track_sector_make_inner(uint16_t wall);
void undo_track_wall_make_first(uint16_t wall, uint16_t portal_wall);
void undo_track_sector_join(uint16_t sector0, uint16_t sector1);
void undo_track_sprite_add(uint16_t sprite);
void undo_track_wall_delete(uint16_t wall);

#endif
