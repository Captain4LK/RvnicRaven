/*
RvnicRaven

Written in 2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_GUI_H_

#define _RVR_GUI_H_

typedef void (*RvR_gui_main_loop)(void);

typedef struct HLH_gui_rvr HLH_gui_rvr;

HLH_gui_rvr *HLH_gui_rvr_create(HLH_gui_element *parent, uint64_t flags, RvR_gui_main_loop main_loop);

#endif
