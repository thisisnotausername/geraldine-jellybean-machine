#ifndef         __SCREENWIPE_H
    #define     __SCREENWIPE_H
    #include    "common.h"

    #define SCRNWIPE_SLIDE_SPEED            15
    #define SCRNWIPE_STATE_IDLE             0
    #define SCRNWIPE_STATE_ACTIVE           99
    #define SCRNWIPE_STATE_SCROLLING_IN     1
    #define SCRNWIPE_STATE_SCROLLING_OUT    2

    /*!
     * @file This is for transitioning away from menus and into gameplay,
     * and areas in gameplay where we may need extra time to load music, enemies
     * or whatever else...
     *
     * Currently, it implements a horizontally-sliding wipe; some sort of radial
     * hole/mask thing should be added.
     */

    extern volatile int scrnwipe_state;
    extern int scrnwipe_direction;
    void    SCRNWIPE_load(void);
    void    SCRNWIPE_unload(void);
    void    SCRNWIPE_start_wipe_in(void);
    void    SCRNWIPE_start_wipe_out(void);
    void    SCRNWIPE_tick(void);
    void    SCRNWIPE_draw(void);
#endif
