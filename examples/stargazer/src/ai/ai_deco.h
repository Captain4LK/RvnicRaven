/*
RvnicRaven - stargazer

Written in 2022 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _AI_DECO_H_

#define _AI_DECO_H_

void ai_deco_init(Entity *e, const uint32_t extra[3]);
void ai_deco_free(Entity *e);
void ai_deco_run(Entity *e);

#endif
