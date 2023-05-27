/*
RvnicRaven - stargazer

Written in 2022,2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>. 
*/

#ifndef _CONFIG_H_

#define _CONFIG_H_

#define CAMERA_COLL_HEIGHT_BELOW (800*64)
#define CAMERA_COLL_RADIUS (256*64)
#define CAMERA_COLL_STEP_HEIGHT (65536/4)
#define GRAVITY (16*64)
#define MESSAGE_MAX 5
#define MESSAGE_TIMEOUT (RvR_fps()*4)

#define CAMERA_SHEAR_SPEED 3
#define CAMERA_SHEAR_MAX 1024
#define CAMERA_SHEAR_MAX_PIXELS ((CAMERA_SHEAR_MAX*RvR_yres())/1024)
#define CAMERA_SHEAR_STEP_FRAME ((RvR_yres()*CAMERA_SHEAR_SPEED)/(RvR_fps()*4))

#define MAX_VERTICAL_SPEED (256*64)
#define JUMP_SPEED (128*64)

#define CARD_RADIUS 64

#define FADE_TIME 15

extern RvR_key config_move_forward;
extern RvR_key config_move_backward;
extern RvR_key config_strafe_left;
extern RvR_key config_strafe_right;
extern RvR_key config_jump;
extern RvR_key config_use;
extern RvR_key config_inventory;

void config_read(const char *path);
void config_write(const char *path);

#endif
