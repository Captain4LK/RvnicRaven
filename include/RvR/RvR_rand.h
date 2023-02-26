/*
RvnicRaven - random number generators

Written in 2023 by Lukas Holzbeierlein (Captain4LK) email: captain4lk [at] tutanota [dot] com

To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.

You should have received a copy of the CC0 Public Domain Dedication along with this software. If not, see <http://creativecommons.org/publicdomain/zero/1.0/>.
*/

#ifndef _RVR_RAND_H_

#define _RVR_RAND_H_

typedef uint64_t RvR_rand_pcg[2];
typedef uint32_t RvR_rand_well[17];
typedef uint32_t RvR_rand_game[2];
typedef uint64_t RvR_rand_xor[2];

void     RvR_rand_pcg_seed(RvR_rand_pcg *pcg, uint32_t seed);
uint32_t RvR_rand_pcg_next(RvR_rand_pcg *pcg);
int32_t  RvR_rand_pcg_next_range(RvR_rand_pcg *pcg, int32_t min, int32_t max);

void     RvR_rand_well_seed(RvR_rand_well *well, uint32_t seed);
uint32_t RvR_rand_well_next(RvR_rand_well *well);
int32_t  RvR_rand_well_next_range(RvR_rand_well *well, int32_t min, int32_t max);

void     RvR_rand_game_seed(RvR_rand_game *game, uint32_t seed);
uint32_t RvR_rand_game_next(RvR_rand_game *game);
int32_t  RvR_rand_game_next_range(RvR_rand_game *game, int32_t min, int32_t max);

void RvR_rand_xor_seed(RvR_rand_xor * xor, uint64_t seed);
uint64_t RvR_rand_xor_next(RvR_rand_xor * xor);
int32_t RvR_rand_xor_next_range(RvR_rand_xor * xor, int32_t min, int32_t max);

#endif
