/*
RvnicRaven retro game engine

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _CONFIG_H_

#define _CONFIG_H_

#define CAMERA_SHEAR_SPEED 3
#define CAMERA_SHEAR_MAX (512 * 64)

#define UNDO_BUFFER_SIZE (1 << 21)
#define TEXTURE_MRU_SIZE (1 << 7)

#define CAMERA_COLL_RADIUS (256 * 64)
#define CAMERA_COLL_HEIGHT_BELOW (800 * 64)
#define CAMERA_COLL_HEIGHT_ABOVE (200 * 64)
#define CAMERA_COLL_STEP_HEIGHT (65536 / 4)

#endif
