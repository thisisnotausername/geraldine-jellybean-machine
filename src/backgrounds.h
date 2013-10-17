/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/

#ifndef      __BACKGROUNDS_H
    #define  __BACKGROUNDS_H

    void BKGRND_load(void);
    void BKGRND_unload(void);
    void BKGRND_tick(void);
    void BKGRND_draw(void);
    void BKGRND_set_next(int which);
    
    void BKGRND_reset_stars(void);
    void BKGRND_tick_stars(void);
    void BKGRND_draw_stars(void);
    void BKGRND_tick_galaxy(void);

#endif
