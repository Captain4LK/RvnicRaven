# RvR_rand_game

State for game random number generator. Created by [mattiasgustavsson](https://mattiasgustavsson.com/), orginally based on [this](https://web.archive.org/web/20160310112827/http://www.redditmirror.cc/cache/websites/mjolnirstudios.com_7yjlc/mjolnirstudios.com/IanBullard/files/79ffbca75a75720f066d491e9ea935a0-10.html) article, based on an IOTD on [flipcode](http://www.flipcode.com/archives/07-15-2002.shtml). 

## Definition

```c
typedef uint32_t RvR_rand_game[2];
```

## Related functions

[RvR_rand_game_seed(RvR_rand_game *game, uint32_t seed);](/rvr/rvr/rand_game_seed)

[RvR_rand_game_next(RvR_rand_game *game);](/rvr/rvr/rand_game_next)

[RvR_rand_game_next_range(RvR_rand_game *game, int32_t min, int32_t max);](/rvr/rvr/rand_game_next_range)
