/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/

#ifndef      __MOUSE_EFFECTS_H
    #define  __MOUSE_EFFECTS_H

    void MEFX_load(void);
    void MEFX_reset(float x, float y);
    void MEFX_spawn_shockring(float x, float y);
    void MEFX_tick(float cursor_x, float cursor_y);
    void MEFX_draw(void);
    void MEFX_unload(void);
#endif
