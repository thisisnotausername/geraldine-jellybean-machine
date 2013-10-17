/*******************************************************************************
*
* Part of GAME NAME - see common.h for copyright info.
*
*******************************************************************************/

#ifndef         __ENTITYGFX_H
    #define     __ENTITYGFX_H

    #include    "common.h"

    #define     MAX_FRAMES          16
    #define     MAX_ACTIONS         7

    #define     ACTION_IDLE         0
    #define     ACTION_WALK_LEFT    1
    #define     ACTION_WALK_RIGHT   2
    #define     ACTION_WALK_UP      3
    #define     ACTION_WALK_DOWN    4
    #define     ACTION_WIN          5
    #define     ACTION_LOSE         6

    typedef struct
    {
        GLuint  frame_data[MAX_ACTIONS][MAX_FRAMES];
        uint8_t frame_counts[MAX_ACTIONS];
    } ENTITY_GFX;

    void ENTGFX_load(const char *foldername, ENTITY_GFX *dest);
    void ENTGFX_unload(ENTITY_GFX *dest);
    void ENTGFX_draw(const ENTITY_GFX *src, float x, float y, int frame, int action);

    void ENTGFX_load_all(void);
    void ENTGFX_unload_all(void);

    extern ENTITY_GFX   entgfx_princess;
    extern ENTITY_GFX   entgfx_monster;
#endif
