#include    "common.h"
#include    "particles.h"
#include    "announcements.h"
#include    "round_start_anim.h"

#ifndef     __PUZGRID_H
   #define  __PUZGRID_H

    #define  PUZGRID_SIZE               15
    #define  PUZGRID_NUM_COLOURS        6
    #define  PUZGRID_TILE_SIZE_IN_PX    32
    #define  PUZGRID_BORDER_SIZE_IN_PX  512
    #define  PUZGRID_BORDER_WIDTH_PX    16

    #define  PUZGRID_EASY               4 //0 // 4
    #define  PUZGRID_MEDIUM             5 //1 // 5
    #define  PUZGRID_HARD               6 //2 // 6 - wtf were these set like this for...? (they're modulos to rand() calls, please don't change)

    #define  PUZGRID_EMPTY              0
    #define  PUZGRID_WALL               1
    #define  PUZGRID_PLAYERSTART        97
    #define  PUZGRID_MONSTERSTART       98
    #define  PUZGRID_EXIT               99

    #define  PUZGRID_PLAYER             127

    #define  PUZGRID_ANIM_NONE          0
    #define  PUZGRID_ANIM_UP            1
    #define  PUZGRID_ANIM_LEFT          2
    #define  PUZGRID_ANIM_RIGHT         3
    #define  PUZGRID_ANIM_DOWN          4

    #define  PUZGRID_ANIM_CLOCK_START_VAL    -15

    typedef int8_t PUZZLE_GRID[PUZGRID_SIZE][PUZGRID_SIZE];

    int puzgrid_check_player_at_exit(void);
    void puzgrid_set_difficulty_level(int level);
    int  puzgrid_get_difficulty_level(void);
    void puzgrid_animate(void);
    void puzgrid_reset_reticle(void);
    void puzgrid_move_reticle(int cursor_x, int cursor_y);
    void puzgrid_rotate_reticled_gems(int left_or_right);
    void puzgrid_draw_reticle(void);
    void puzgrid_load_assets(void);
    void puzgrid_unload_assets(void);
    void puzgrid_draw(int x_offset, int y_offset);
    void puzgrid_load_map(const char *filename);
    void puzgrid_randomize_puzzle(void);
    int puzgrid_swap_gems(int from_x, int from_y, int direction);
    int puzgrid_check_for_matches(void);
    int puzgrid_collapse_gems(void);
    int puzgrid_remove_matched_gems(void);
    void puzgrid_shine_tick(void);
    void puzgrid_draw_exit_sign(int x, int y);

    extern int puzgrid_combo_length;
    extern int puzgrid_num_colours_matched;
    extern int puzgrid_multi_directions_matched;
    extern int puzgrid_square_matched;
    extern int puzgrid_animation_clock;

#endif
