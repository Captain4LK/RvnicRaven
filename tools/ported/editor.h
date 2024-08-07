/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _EDITOR_H_

#define _EDITOR_H_

extern RvR_port_cam camera;

void editor_init(void);
void editor_update(void);
void editor_draw(void);

void camera_update();
void camera_update_collision();

void editor_set_3d(void);
void editor_set_2d(void);

#endif
