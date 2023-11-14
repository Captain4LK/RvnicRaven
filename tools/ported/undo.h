/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

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
void undo_track_wall_move(int16_t wall, RvR_fix22 px, RvR_fix22 py);
void undo_track_sector(int16_t sector);

#endif
