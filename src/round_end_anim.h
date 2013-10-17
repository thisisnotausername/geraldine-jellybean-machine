#ifndef      __ROUND_END_ANIM_H
    #define  __ROUND_END_ANIM_H

    #include "common.h"
    #include "particles.h"
    #include "gameclock.h"

    #define  RNDEND_WIN      1
    #define  RNDEND_LOSE     2

    void RNDEND_load_assets(void);
    void RNDEND_unload_assets(void);
    void RNDEND_reset(void);
    void RNDEND_tick(void);
    void RNDEND_paint(void);
    void RNDEND_start(int type);

    extern uint32_t RNDEND_done;

#endif // __ROUND_END_ANIM_H
