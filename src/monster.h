#include        "common.h"
#include        "entity_gfx.h" 
#include        "puzgrid.h"

#ifndef         __MONSTER_H
    #define     __MONSTER_H
    
    #define     MAX_MONSTERS                24
    #define     MAX_MONSTER_SLOTS_TO_WALK   12
    
    typedef struct
    {
        int x; 
        int y;
        int frame;
        int tween;
        int alive; ///!< Zero for no, anything else for yes.
    } MONSTER;
    
    extern MONSTER monster_pool[MAX_MONSTERS];
    extern int monster_num_active;
    
    void MONSTER_spawn(int x, int y);
    void MONSTER_think_all(int player_x, int player_y);
    void MONSTER_kill_at(int x, int y);
    void MONSTER_draw_all(void);
#endif
