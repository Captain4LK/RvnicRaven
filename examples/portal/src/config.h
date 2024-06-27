/*
RvnicRaven - portal

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _CONFIG_H_

#define _CONFIG_H_

#define BOOKCASE_COUNT 24
#define BOOK_INVALID (UINT16_MAX)

extern RvR_key config_move_forward;
extern RvR_key config_move_backward;
extern RvR_key config_strafe_left;
extern RvR_key config_strafe_right;
extern RvR_key config_jump;
extern RvR_key config_use;
extern RvR_key config_crouch;

void config_read(const char *path);
void config_write(const char *path);

#endif
