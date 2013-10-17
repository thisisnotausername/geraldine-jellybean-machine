#ifndef         __UNLOCKS_H
    #define     __UNLOCKS_H

    #include    "common.h"

    #define     NUM_UNLOCK_DIFFICULTIES     3

    #define     UNLOCKS_EASY                0
    #define     UNLOCKS_MEDIUM              1
    #define     UNLOCKS_HARD                2

    extern char unlockables[NUM_UNLOCK_DIFFICULTIES];

    void UNLOCKED_load(void);
    void UNLOCKED_save(void);

#endif
