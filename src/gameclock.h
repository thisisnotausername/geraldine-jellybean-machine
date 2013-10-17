/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/

#include "common.h"

#ifndef      __GAMECLOCK_H
    #define  __GAMECLOCK_H

    void GMCLK_load(void);
    void GMCLK_unload(void);
    void GMCLK_tick(void);
    void GMCLK_draw(void);
    void GMCLK_reset(int amount);
    void GMCLK_add(int amount);
    int GMCLK_get_time_remaining(void);
    int GMCLK_get_total_time_elapsed(void);

    extern int GMCLK_should_count_down;
#endif
