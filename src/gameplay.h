#ifndef      __GAMEPLAY_H
    #define  __GAMEPLAY_H

    #include "common.h"
    #include "puzgrid.h"
    #include "mouse_effects.h"
    #include "particles.h"
    #include "screenwipe.h"
    #include "announcements.h"
    #include "gameclock.h"
    #include "round_end_anim.h"
    #include "round_start_anim.h"
    #include "unlocks.h"

    void GAMEPLAY_init();
    void GAMEPLAY_load();
    void GAMEPLAY_unload();
    void GAMEPLAY_tick();
    void GAMEPLAY_draw();

    extern int gameplay_curr_level;

#endif
