/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/
#include     "common.h"
#include     "particles.h"

#ifndef      __ANNOUNCEMENTS_H
    #define  __ANNOUNCEMENTS_H

    #define  ANNOUNCEMENT_MINUS_TENTH_SEC   1
    #define  ANNOUNCEMENT_MINUS_FOUR        2
    #define  ANNOUNCEMENT_COMBO             3
    #define  ANNOUNCEMENT_2_COLS            4
    #define  ANNOUNCEMENT_2_DIRS            5
    #define  ANNOUNCEMENT_SQUARE            6
    #define  ANNOUNCEMENT_4_OR_MORE         7

    /*!
     * @file This module grew by accretion and as such, has hilariously-bad code.
     * I am sorry.  At this point, I'm mostly anxious to get the game into a playable,
     * releaseable state.  I'll clean it up later.
     */

    void ANCMT_reset(void);
    void ANCMT_load(void);
    void ANCMT_unload(void);
    void ANCMT_tick(void);
    void ANCMT_draw(void);
    void ANCMT_spawn(int which, int combo_length, int added_time);
    void ANCMT_spawn_time_lost(int one_or_four);
#endif
