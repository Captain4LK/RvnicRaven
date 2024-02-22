/*
RvnicRaven - Portal renderer config

Written in 2023,2024 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_PORT_CONFIG_H_

#define _RVR_PORT_CONFIG_H_

//If the RVR_PORT_PIXELKEY button is pressed, the renderer
//redraws every framebuffer write, showing how the scene
//is rendered pixel by pixel
#define RVR_PORT_PIXELBYPIXEL 1
#define RVR_PORT_PIXELKEY RVR_KEY_M

//Span length for sub-affine slope rendering
#define RVR_PORT_SPAN 8

//(1<<Number) of slots in plane hashtable
#define RVR_PORT_PLANE_SLOTS 8

#endif
