#include "puzgrid.h"

#define WALL_GFX_PATH           "./gfx/puzzle/wall.tga"
#define EXIT_SIGN_GFX_PATH      "./gfx/puzzle/exit-sign.tga"
#define EXIT_DOOR_GFX_PATH      "./gfx/puzzle/exit-door.tga"
#define BORDER_GFX_PATH         "./gfx/puzzle/puzzle-border.tga"
#define BORDER_BACK_GFX_PATH    "./gfx/puzzle/puzzle-back.tga"
#define GEM_GFX_PATH            "./gfx/puzzle/%02d.tga"
#define RETICLE_GFX_PATH        "./gfx/puzzle/reticle/reticle.tga"
#define PRINCESS_GFX_IDLE_PATH  "./gfx/hero/idle%02d.tga"

#define HEART_SPEED         ((rand() % 27 - 13) / 3.75) //!< initial speed for particles
#define HEART_INTENSITY     ((rand() % 32) + 223)      //!< initial intensity for particles

#define PRINCESS_FRAMES         24
#define PRINCESS_FRAME_WIDTH    32
#define PRINCESS_FRAME_HEIGHT   64
#define PRINCESS_HEIGHT_FIXUP   28
#define PRINCESS_FRAME_HOLD     4

/**************************************************************************************************/

#define     SHINE_GFX_PATH          "./gfx/puzzle/shine/shine%02d.tga"
#define     NUM_SHINE_FRAMES        12
#define     SHINE_ANIM_CLOCK_MIN    100
#define     SHINE_ANIM_CLOCK_MAX    200
GLTEXTURE   puzgrid_shine_gfx[NUM_SHINE_FRAMES];
int         puzgrid_shine_anim_clock = SHINE_ANIM_CLOCK_MIN;
int         puzgrid_shine_anim_colour = 1;

/**************************************************************************************************/

GLTEXTURE  puzgrid_princess_idle_gfx[PRINCESS_FRAMES];
//BITMAP  *puzgrid_princess_win[PRINCESS_FRAMES];

GLTEXTURE   puzgrid_reticle_gfx  = 0;
GLTEXTURE   puzgrid_player_gfx   = 0;
GLTEXTURE   puzgrid_exit_gfx     = 0;
GLTEXTURE   puzgrid_exit_sign_gfx= 0;
GLTEXTURE   puzgrid_wall_gfx     = 0;
GLTEXTURE   puzgrid_border_gfx   = 0;
GLTEXTURE   puzgrid_borderbg_gfx = 0;
GLTEXTURE   puzgrid_gem_gfx[PUZGRID_NUM_COLOURS];

SAMPLE  *puzgrid_match1_smp = NULL;
SAMPLE  *puzgrid_match2_smp = NULL;
SAMPLE  *puzgrid_match3_smp = NULL;
SAMPLE  *puzgrid_match4_smp = NULL;
SAMPLE  *puzgrid_match5_smp = NULL;
SAMPLE  *puzgrid_rotate_smp = NULL;

PUZZLE_GRID puzgrid_gems;
PUZZLE_GRID puzgrid_remove;
PUZZLE_GRID puzgrid_walls;
PUZZLE_GRID puzgrid_matches_last_check;

int puzgrid_combo_length = 0;
int puzgrid_num_colours_matched = 0;
int puzgrid_multi_directions_matched = 0;
int puzgrid_square_matched = 0;
int puzgrid_four_or_more_matched = 0;

/*!
 * @brief An array that holds where the blocks in the current gem grid
 * should be animated to.
 */
PUZZLE_GRID puzgrid_animations;
PUZZLE_GRID puzgrid_gems_cached;

int puzgrid_difficulty = PUZGRID_EASY;
int puzgrid_max_monsters = 0;
int puzgrid_turns_between_monsters = 0;

int puzgrid_reticle_x;
int puzgrid_reticle_y;

int puzgrid_animation_clock;
int puzgrid_tile_anim_clock;

int PUZGRID_BORDER_HPOS;
int PUZGRID_BORDER_VPOS;
int PUZGRID_GEM_HPOS;
int PUZGRID_GEM_VPOS;

int puzgrid_cursor_is_inside;

/**************************************************************************************************/

int puzgrid_check_player_at_exit(void)
{
    int reached_exit = FALSE;

    int x_index;
    int y_index;

    int player_x;
    int player_y;

    int exit_x;
    int exit_y;

    // find the player and the exit
    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if(puzgrid_gems[y_index][x_index] == PUZGRID_PLAYER)
            {
                player_x = x_index;
                player_y = y_index;
            }

            if(puzgrid_walls[y_index][x_index] == PUZGRID_EXIT)
            {
                exit_x = x_index;
                exit_y = y_index;
            }
        }
    }

    // compare the two - if the player is in an adjacent square to the exit,
    // then the player has won the stage...
    if (
        ((player_x  == exit_x) && (((player_y - 1) == exit_y) || ((player_y + 1) == exit_y))) ||
        ((player_y  == exit_y) && (((player_x - 1) == exit_x) || ((player_x + 1) == exit_x))) )
    {
        reached_exit = TRUE;
    }

    return reached_exit;
}

/**************************************************************************************************/

int puzgrid_get_difficulty_level(void)
{
    return puzgrid_difficulty - PUZGRID_EASY;
}

/**************************************************************************************************/

void puzgrid_set_difficulty_level(int level)
{
    switch(level)
    {
        case PUZGRID_EASY:
        case PUZGRID_MEDIUM:
        case PUZGRID_HARD:
            puzgrid_difficulty = level;
        return;

        default:
            DUH_WHERE_AM_I("Invalid difficulty level; setting to sensible default.");
            puzgrid_difficulty = PUZGRID_MEDIUM;
    }
}

/**************************************************************************************************/
/*!
 * @brief Animate the shimmer effect across the face of the gems.
 */
void puzgrid_shine_tick(void)
{
    static int hold_timer = 0;

    hold_timer++;

    if(hold_timer & 1)
    {
        puzgrid_shine_anim_clock--;
        if(puzgrid_shine_anim_clock < 0)
        {
            puzgrid_shine_anim_clock =
                (rand() % (SHINE_ANIM_CLOCK_MAX - SHINE_ANIM_CLOCK_MIN)) + SHINE_ANIM_CLOCK_MIN;

            puzgrid_shine_anim_colour = rand() % puzgrid_difficulty;
        }
    }
}

/**************************************************************************************************/
/*!
 * @brief Animate the falling gems.
 */
void puzgrid_animate(void)
{
    puzgrid_shine_tick();
    if(puzgrid_animation_clock < 0) puzgrid_animation_clock++;

    if(puzgrid_animation_clock == 0)
    {
        int x_index, y_index;
        for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
        {
            for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
            {
                puzgrid_animations[y_index][x_index] = PUZGRID_ANIM_NONE;
            }
        }
    }
}

/**************************************************************************************************/

void puzgrid_reset_reticle(void)
{
   puzgrid_reticle_x = PUZGRID_SIZE / 2;
   puzgrid_reticle_y = PUZGRID_SIZE / 2;
}

/**************************************************************************************************/

void puzgrid_move_reticle(int cursor_x, int cursor_y)
{
    cursor_x -= PUZGRID_GEM_HPOS;
    cursor_y -= PUZGRID_GEM_VPOS;
    puzgrid_cursor_is_inside = TRUE;


    int new_reticle_x = cursor_x / PUZGRID_TILE_SIZE_IN_PX;
    int new_reticle_y = cursor_y / PUZGRID_TILE_SIZE_IN_PX;

    if((new_reticle_x < 1) || (new_reticle_y < 1) || (new_reticle_x > PUZGRID_SIZE - 1) || (new_reticle_y > PUZGRID_SIZE - 1))
    {
        // clamp this, yo
        if (new_reticle_x < 1) puzgrid_reticle_x = 1;
        if (new_reticle_y < 1) puzgrid_reticle_y = 1;
        if (new_reticle_x > PUZGRID_SIZE - 1) puzgrid_reticle_x = PUZGRID_SIZE - 1;
        if (new_reticle_y > PUZGRID_SIZE - 1) puzgrid_reticle_y = PUZGRID_SIZE - 1;

        puzgrid_cursor_is_inside = FALSE;
        return;
    }

    puzgrid_reticle_x = new_reticle_x;
    puzgrid_reticle_y = new_reticle_y;


    /// TODO: if we actually moved, play clicking noise
}

/**************************************************************************************************/
/*!
 * @brief 0 for clockwise, nonzero for anticlockwise
 *
 * @note: to move gems, set them like they're supposed to be in puzgrid_gems, then
 *    set up the animations in puzgrid_animations.
 */
void puzgrid_rotate_reticled_gems(int left_or_right)
{
    uint8_t tmp;

    if(puzgrid_animation_clock < 0) return;

    // check if any of the squares inside the reticle contain a wall -
    // if so, then this isn't a legal move.
    if
    ((!puzgrid_cursor_is_inside) ||
       ( ((puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] == PUZGRID_WALL) ||
         (puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x]     == PUZGRID_WALL) ||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x - 1]     == PUZGRID_WALL)||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x]         == PUZGRID_WALL)) ||
        ((puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] == PUZGRID_EXIT) ||
         (puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x]     == PUZGRID_EXIT) ||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x - 1]     == PUZGRID_EXIT)||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x]         == PUZGRID_EXIT)) )
    )
    {
        /// TODO: play a sad noise here and add some camera shake
        return;
    }

    puzgrid_combo_length = 0;
    puzgrid_animation_clock = PUZGRID_ANIM_CLOCK_START_VAL;

    // check if the princess is in the reticle; if she is, it costs more time
    if  ((puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] == PUZGRID_PLAYER) ||
         (puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x]     == PUZGRID_PLAYER) ||
         (puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x - 1]     == PUZGRID_PLAYER) ||
         (puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x]         == PUZGRID_PLAYER))
    {
        ANCMT_spawn_time_lost(TRUE);
        GMCLK_add(-3);
    }
    else
    {
        ANCMT_spawn_time_lost(FALSE);
    }

    if(left_or_right)
    {
        tmp = puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1];
        puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] = puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x - 1];
        puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x - 1] = puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x];
        puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x] = puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x];
        puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x] = tmp;

        puzgrid_animations[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1]  = PUZGRID_ANIM_UP;
        puzgrid_animations[puzgrid_reticle_y][puzgrid_reticle_x - 1]      = PUZGRID_ANIM_LEFT;
        puzgrid_animations[puzgrid_reticle_y][puzgrid_reticle_x]          = PUZGRID_ANIM_DOWN;
        puzgrid_animations[puzgrid_reticle_y - 1][puzgrid_reticle_x]      = PUZGRID_ANIM_RIGHT;
    }
    else
    {
        tmp = puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1];
        puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] = puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x];
        puzgrid_gems[puzgrid_reticle_y - 1][puzgrid_reticle_x] = puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x];
        puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x] = puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x - 1];
        puzgrid_gems[puzgrid_reticle_y][puzgrid_reticle_x - 1] = tmp;

        puzgrid_animations[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1]  = PUZGRID_ANIM_LEFT;
        puzgrid_animations[puzgrid_reticle_y - 1][puzgrid_reticle_x]      = PUZGRID_ANIM_UP;
        puzgrid_animations[puzgrid_reticle_y][puzgrid_reticle_x]          = PUZGRID_ANIM_RIGHT;
        puzgrid_animations[puzgrid_reticle_y][puzgrid_reticle_x - 1]      = PUZGRID_ANIM_DOWN;
    }

    play_sample(puzgrid_rotate_smp, 255, 128, 1000, 0);
}

/**************************************************************************************************/

void puzgrid_draw_exit_sign(int x, int y)
{
    static int exit_hue;
    int r, g, b;

    exit_hue++;

    hsv_to_rgb((exit_hue % 400) * (360.0f / 400.0f), 0.25f, 1.0f, &r, &g, &b);

    glEnable(GL_BLEND);
    glColor4ub(r,g,b,127 + (sin(exit_hue / 385.0f) * 32));
    COMMON_draw_sprite(puzgrid_exit_sign_gfx,
    x - (48),
    y - (62) + (sin(exit_hue / 40.0f) * 5),0,
        128, 64);

    glDisable(GL_BLEND);
    glColor4f(1, 1, 1, 1);
}

/**************************************************************************************************/
/*!
 * @brief Swaps the positions of two gems, if permitted, in the specified direction.  Used
 * for monster movement.
 *
 * @note Returns TRUE if this move is allowed and is going to be carried out or FALSE
 * otherwise.
 */
int puzgrid_swap_gems(int from_x, int from_y, int direction)
{
    uint8_t tmp;

    // not permitted during the animation phase - monsters
    // shouldn't be thinking then anyway.
    if(puzgrid_animation_clock < 0) return FALSE;

    // check whether we're trying to travel into a wall; if not
    // make the move.
    switch (direction)
    {
        case PUZGRID_ANIM_UP :
            // wall test - if it fails, this move is invalid and the caller needs to know.
            if ((from_y < 1) || (puzgrid_walls[from_y - 1][from_x] != PUZGRID_EMPTY))
                return FALSE;

            // swap
            tmp = puzgrid_gems[from_y][from_x];
            puzgrid_gems[from_y][from_x] = puzgrid_gems[from_y - 1][from_x];
            puzgrid_gems[from_y - 1][from_x] = tmp;

            // animate
            puzgrid_animations[from_y - 1][from_x]  = PUZGRID_ANIM_DOWN;
            puzgrid_animations[from_y][from_x]      = PUZGRID_ANIM_UP;
        break;

        case PUZGRID_ANIM_DOWN :
            // wall test
            if ((from_y > PUZGRID_SIZE - 2) || (puzgrid_walls[from_y + 1][from_x] != PUZGRID_EMPTY))
                return FALSE;

            // swap
            tmp = puzgrid_gems[from_y][from_x];
            puzgrid_gems[from_y][from_x] = puzgrid_gems[from_y + 1][from_x];
            puzgrid_gems[from_y + 1][from_x] = tmp;

            // animate
            puzgrid_animations[from_y + 1][from_x]  = PUZGRID_ANIM_UP;
            puzgrid_animations[from_y][from_x]      = PUZGRID_ANIM_DOWN;
        break;

        case PUZGRID_ANIM_LEFT :
            // wall test
            if ((from_x < 1) || (puzgrid_walls[from_y][from_x - 1] != PUZGRID_EMPTY))
                return FALSE;

            // swap
            tmp = puzgrid_gems[from_y][from_x];
            puzgrid_gems[from_y][from_x] = puzgrid_gems[from_y][from_x - 1];
            puzgrid_gems[from_y][from_x - 1] = tmp;

            // animate
            puzgrid_animations[from_y][from_x - 1]  = PUZGRID_ANIM_RIGHT;
            puzgrid_animations[from_y][from_x]      = PUZGRID_ANIM_LEFT;
        break;

        case PUZGRID_ANIM_RIGHT :
            // wall test
            if ((from_x > PUZGRID_SIZE - 2) || (puzgrid_walls[from_y][from_x + 1] != PUZGRID_EMPTY))
                return FALSE;

            // swap
            tmp = puzgrid_gems[from_y][from_x];
            puzgrid_gems[from_y][from_x] = puzgrid_gems[from_y][from_x + 1];
            puzgrid_gems[from_y][from_x + 1] = tmp;

            // animate
            puzgrid_animations[from_y][from_x + 1]  = PUZGRID_ANIM_LEFT;
            puzgrid_animations[from_y][from_x]      = PUZGRID_ANIM_RIGHT;
        break;
    }

    // this was a valid move.
    return TRUE;
}

/**************************************************************************************************/

void puzgrid_draw_reticle(void)
{
    static int reticle_hue;
    int r, g, b;
    glEnable(GL_BLEND);

    // don't paint this if we can't actually play yet
    if (!RNDSTRT_done) return;

    if     ((!puzgrid_cursor_is_inside) ||
       ( ((puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] == PUZGRID_WALL) ||
         (puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x]     == PUZGRID_WALL) ||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x - 1]     == PUZGRID_WALL)||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x]         == PUZGRID_WALL)) ||
        ((puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x - 1] == PUZGRID_EXIT) ||
         (puzgrid_walls[puzgrid_reticle_y - 1][puzgrid_reticle_x]     == PUZGRID_EXIT) ||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x - 1]     == PUZGRID_EXIT)||
         (puzgrid_walls[puzgrid_reticle_y][puzgrid_reticle_x]         == PUZGRID_EXIT)) )
    )
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // colour animation
        reticle_hue += 1;

        // in an invalid area - draw with reduced saturation
        // inner square
        hsv_to_rgb(reticle_hue % 360, 0.2f, 0.6f, &r, &g, &b);

    }
    else
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        // colour animation
        reticle_hue += 2;

        // in an allowed area - draw normally
        // inner square
        hsv_to_rgb(reticle_hue % 360, 0.6f, 1, &r, &g, &b);
    }

    glColor3ub(r, g, b);

    COMMON_draw_sprite(puzgrid_reticle_gfx,
        (puzgrid_reticle_x) * PUZGRID_TILE_SIZE_IN_PX + PUZGRID_BORDER_HPOS - PUZGRID_BORDER_WIDTH_PX - 1.5,
        (puzgrid_reticle_y) * PUZGRID_TILE_SIZE_IN_PX + PUZGRID_BORDER_VPOS - PUZGRID_BORDER_WIDTH_PX - 1.5, 1,
        PUZGRID_TILE_SIZE_IN_PX * 2 + 3, PUZGRID_TILE_SIZE_IN_PX * 2 + 3);

    glDisable(GL_BLEND);

}

/**************************************************************************************************/

void puzgrid_load_assets(void)
{
    // Load the puzzle sounds, creating a 'dummy' one if a file is missing.
    if(exists("./snd/rotate.wav"))
        puzgrid_rotate_smp = load_sample("./snd/rotate.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/rotate.wav");
        puzgrid_rotate_smp = create_sample(8, 0, 1, 1);
    }

    if(exists("./snd/match1.wav"))
        puzgrid_match1_smp = load_sample("./snd/match1.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/match1.wav");
        puzgrid_match1_smp = create_sample(8, 0, 1, 1);
    }

    if(exists("./snd/match2.wav"))
        puzgrid_match2_smp = load_sample("./snd/match2.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/match2.wav");
        puzgrid_match2_smp = create_sample(8, 0, 1, 1);
    }

    if(exists("./snd/match3.wav"))
        puzgrid_match3_smp = load_sample("./snd/match3.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/match3.wav");
        puzgrid_match3_smp = create_sample(8, 0, 1, 1);
    }

    if(exists("./snd/match4.wav"))
        puzgrid_match4_smp = load_sample("./snd/match4.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/match4.wav");
        puzgrid_match4_smp = create_sample(8, 0, 1, 1);
    }

    if(exists("./snd/match5.wav"))
        puzgrid_match5_smp = load_sample("./snd/match5.wav");
    else
    {
        DUH_WHERE_AM_I("missing sound: ./snd/match5.wav");
        puzgrid_match5_smp = create_sample(8, 0, 1, 1);
    }

    // Load the images, creating a 'dummy' one if a file is missing.
    int index;
    char buf[1024];

    ///===========================

    puzgrid_wall_gfx        = COMMON_load_texture(WALL_GFX_PATH);
    puzgrid_exit_gfx        = COMMON_load_texture(EXIT_DOOR_GFX_PATH);
    puzgrid_exit_sign_gfx   = COMMON_load_texture(EXIT_SIGN_GFX_PATH);
    puzgrid_border_gfx      = COMMON_load_texture(BORDER_GFX_PATH);
    puzgrid_borderbg_gfx    = COMMON_load_texture(BORDER_BACK_GFX_PATH);
    puzgrid_reticle_gfx     = COMMON_load_texture(RETICLE_GFX_PATH);

    for(index = 0; index < PUZGRID_NUM_COLOURS; index++)
    {
        snprintf(buf, 1023, GEM_GFX_PATH, index + 1);
        puzgrid_gem_gfx[index] = COMMON_load_texture(buf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

    for(index = 0; index < NUM_SHINE_FRAMES; index++)
    {
        snprintf(buf, 1023, SHINE_GFX_PATH, index + 1);
        puzgrid_shine_gfx[index] = COMMON_load_texture(buf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }

    /// chopping block - princess and monster drawing really
    /// should go into their own source files.
    for(index = 0; index < PRINCESS_FRAMES; index++)
    {
        snprintf(buf, 1023, PRINCESS_GFX_IDLE_PATH, index + 1);
        puzgrid_princess_idle_gfx[index] = COMMON_load_texture(buf);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    }
}

/**************************************************************************************************/

void puzgrid_unload_assets(void)
{
    int index;

    for(index = 0; index < PUZGRID_NUM_COLOURS; index++)
        glDeleteTextures(1, &puzgrid_gem_gfx[index]);

    for(index = 0; index < NUM_SHINE_FRAMES; index++)
        glDeleteTextures(1, &puzgrid_shine_gfx[index]);

    for(index = 0; index < PRINCESS_FRAMES; index++)
        glDeleteTextures(1, &puzgrid_princess_idle_gfx[index]);

    destroy_sample(puzgrid_rotate_smp);
    destroy_sample(puzgrid_match1_smp);
    destroy_sample(puzgrid_match2_smp);
    destroy_sample(puzgrid_match3_smp);
    destroy_sample(puzgrid_match4_smp);
    destroy_sample(puzgrid_match5_smp);

    glDeleteTextures(1, &puzgrid_wall_gfx       );
    glDeleteTextures(1, &puzgrid_exit_gfx       );
    glDeleteTextures(1, &puzgrid_exit_sign_gfx  );
    glDeleteTextures(1, &puzgrid_border_gfx     );
    glDeleteTextures(1, &puzgrid_borderbg_gfx   );
    glDeleteTextures(1, &puzgrid_reticle_gfx    );
}

/**************************************************************************************************/

void puzgrid_draw(int x_offset, int y_offset)
{
    int x_index, y_index;

    static int player_anim_clock;
    static int exit_sign_anim_clock;

    glColor4f(1, 1, 1, 1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    exit_sign_anim_clock++;
    player_anim_clock++;

    int exit_sign_val = 43 + (int)(sin(exit_sign_anim_clock / 41.5f) * 20);

    if(player_anim_clock >= (PRINCESS_FRAMES * PRINCESS_FRAME_HOLD))
    {
        player_anim_clock = 0;
    }

    puzgrid_player_gfx = puzgrid_princess_idle_gfx[player_anim_clock / PRINCESS_FRAME_HOLD];

    // keep our offset vars in sync
    PUZGRID_BORDER_HPOS = x_offset;
    PUZGRID_BORDER_VPOS = y_offset;

    PUZGRID_GEM_HPOS = PUZGRID_BORDER_HPOS + PUZGRID_BORDER_WIDTH_PX;
    PUZGRID_GEM_VPOS = PUZGRID_BORDER_VPOS + PUZGRID_BORDER_WIDTH_PX;

    // paint the border background
    COMMON_draw_sprite(puzgrid_borderbg_gfx, PUZGRID_BORDER_HPOS, PUZGRID_BORDER_VPOS, 3,
                        PUZGRID_BORDER_SIZE_IN_PX, PUZGRID_BORDER_SIZE_IN_PX);

    // paint the puzzle
    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            // does this square contain a wall?
            if(puzgrid_walls[y_index][x_index] != PUZGRID_WALL)
            {
                // is it empty?
                if (puzgrid_gems[y_index][x_index] != -1)
                {
                    // is it the player?
                    if (puzgrid_gems[y_index][x_index] != PUZGRID_PLAYER)
                    {
                        // nope, it's a gemstone
                        switch(puzgrid_animations[y_index][x_index])
                        {
                            case PUZGRID_ANIM_NONE:
                                // draw gem
                                COMMON_draw_sprite(puzgrid_gem_gfx[puzgrid_gems[y_index][x_index]],
                                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX), 2,
                                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                                // draw shine
                                if ((puzgrid_shine_anim_colour == puzgrid_gems[y_index][x_index]) &&
                                    (puzgrid_shine_anim_clock < NUM_SHINE_FRAMES))
                                {
                                    COMMON_draw_sprite(puzgrid_shine_gfx[puzgrid_shine_anim_clock],
                                        PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX), 1,
                                        PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);
                                }
                            break;

                            case PUZGRID_ANIM_DOWN:
                                COMMON_draw_sprite(puzgrid_gem_gfx[puzgrid_gems[y_index][x_index]],
                                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2) , 2,
                                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                                if ((puzgrid_shine_anim_colour == puzgrid_gems[y_index][x_index]) &&
                                    (puzgrid_shine_anim_clock < NUM_SHINE_FRAMES))
                                {
                                    COMMON_draw_sprite(puzgrid_shine_gfx[puzgrid_shine_anim_clock],
                                        PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2) , 2,
                                        PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);
                                }
                            break;

                            case PUZGRID_ANIM_RIGHT:
                                COMMON_draw_sprite(puzgrid_gem_gfx[puzgrid_gems[y_index][x_index]],
                                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2) ,
                                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),  2,
                                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                                if ((puzgrid_shine_anim_colour == puzgrid_gems[y_index][x_index]) &&
                                    (puzgrid_shine_anim_clock < NUM_SHINE_FRAMES))
                                {
                                    COMMON_draw_sprite(puzgrid_shine_gfx[puzgrid_shine_anim_clock],
                                        PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2) ,
                                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),  2,
                                        PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);
                                }
                            break;

                            case PUZGRID_ANIM_UP:
                                COMMON_draw_sprite(puzgrid_gem_gfx[puzgrid_gems[y_index][x_index]],
                                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2),  2,
                                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                                if ((puzgrid_shine_anim_colour == puzgrid_gems[y_index][x_index]) &&
                                    (puzgrid_shine_anim_clock < NUM_SHINE_FRAMES))
                                {
                                    COMMON_draw_sprite(puzgrid_shine_gfx[puzgrid_shine_anim_clock],
                                        PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2),  2,
                                        PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);
                                }
                            break;

                            case PUZGRID_ANIM_LEFT:
                                COMMON_draw_sprite(puzgrid_gem_gfx[puzgrid_gems[y_index][x_index]],
                                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2) ,
                                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),  2,
                                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                                if ((puzgrid_shine_anim_colour == puzgrid_gems[y_index][x_index]) &&
                                    (puzgrid_shine_anim_clock < NUM_SHINE_FRAMES))
                                {
                                    COMMON_draw_sprite(puzgrid_shine_gfx[puzgrid_shine_anim_clock],
                                        PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2) ,
                                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),  2,
                                        PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);
                                }
                            break;
                        }
                    }
                }
            }
        }
    }

    // paint the border
    COMMON_draw_sprite(puzgrid_border_gfx, PUZGRID_BORDER_HPOS, PUZGRID_BORDER_VPOS, 1,
                        PUZGRID_BORDER_SIZE_IN_PX, PUZGRID_BORDER_SIZE_IN_PX);

    // paint walls in a separate pass (hides a bug with newly-spawned blocks
    // appearing in the wrong place; no time to fix it)
    glDisable(GL_BLEND); // not needed, since the wall tiles are solid

    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            // decide how to light the edges of the wall tiles
            float top_left  = 1;
            float top_right = 1;
            float bot_right = 1;
            float bot_left  = 1;

            // There's a clearer, more expressive way to do this, but...
            // compute shadows naively
            if((x_index > 0) && (y_index > 0))
            {
                if(puzgrid_walls[y_index - 1][x_index - 1] == PUZGRID_WALL)
                {
                    top_left = 0.133f;
                }
            }

            if ((x_index > 0) && (y_index < PUZGRID_SIZE-1))
            {
                if(puzgrid_walls[y_index + 1][x_index - 1] == PUZGRID_WALL)
                {
                    bot_left = 0.133f;
                }
            }

            if ((x_index < PUZGRID_SIZE-1) && (y_index > 0))
            {
                if(puzgrid_walls[y_index - 1][x_index + 1] == PUZGRID_WALL)
                {
                    top_right = 0.133f;
                }
            }

            if((x_index < PUZGRID_SIZE-1) && (y_index < PUZGRID_SIZE-1))
            {
                if(puzgrid_walls[y_index + 1][x_index + 1] == PUZGRID_WALL)
                {
                    bot_right = 0.133f;
                }
            }

            // fix up places that shouldn't be shadowed
            if ((x_index > 0) && (puzgrid_walls[y_index][x_index - 1] != PUZGRID_WALL))
            {
                bot_left = 1;
                top_left = 1;
            }

            if ((x_index < PUZGRID_SIZE-1) && (puzgrid_walls[y_index][x_index + 1] != PUZGRID_WALL))
            {
                bot_right = 1;
                top_right = 1;
            }

            if ((y_index > 0) && (puzgrid_walls[y_index - 1][x_index] != PUZGRID_WALL))
            {
                top_left  = 1;
                top_right = 1;
            }

            if ((y_index < PUZGRID_SIZE-1) && (puzgrid_walls[y_index + 1][x_index] != PUZGRID_WALL))
            {
                bot_right = 1;
                bot_left  = 1;
            }


            if(puzgrid_walls[y_index][x_index] == PUZGRID_WALL)
            {
                COMMON_draw_lit_sprite(puzgrid_wall_gfx,
                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX), 1,
                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX,
                    top_left, top_right, bot_right, bot_left);
            }

            if(puzgrid_walls[y_index][x_index] == PUZGRID_EXIT)
            {
                glColor3ub(255, 255, 255);
                COMMON_draw_sprite(puzgrid_exit_gfx,
                    PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX), 1,
                    PUZGRID_TILE_SIZE_IN_PX, PUZGRID_TILE_SIZE_IN_PX);

                // paint the exit sign
                puzgrid_draw_exit_sign(PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX));

            }
        }
    }

    // paint the player
    glEnable(GL_BLEND);
    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if (puzgrid_gems[y_index][x_index] == PUZGRID_PLAYER)
            {
                // it's the player.
                /// CHOPPING BLOCK
                glColor4f(1, 1, 1, 1);
                switch(puzgrid_animations[y_index][x_index])
                {
                    case PUZGRID_ANIM_NONE:
                            COMMON_draw_sprite(puzgrid_player_gfx,
                                PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - PRINCESS_HEIGHT_FIXUP,
                                0, PRINCESS_FRAME_WIDTH, PRINCESS_FRAME_HEIGHT);
                    break;

                    case PUZGRID_ANIM_DOWN:
                            COMMON_draw_sprite(puzgrid_player_gfx,
                                PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2)  - PRINCESS_HEIGHT_FIXUP,
                                0, PRINCESS_FRAME_WIDTH, PRINCESS_FRAME_HEIGHT);
                    break;

                    case PUZGRID_ANIM_RIGHT:
                            COMMON_draw_sprite(puzgrid_player_gfx,
                                PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) + (puzgrid_animation_clock * 2) ,
                                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - PRINCESS_HEIGHT_FIXUP,
                                0, PRINCESS_FRAME_WIDTH, PRINCESS_FRAME_HEIGHT);
                    break;

                    case PUZGRID_ANIM_UP:
                            COMMON_draw_sprite(puzgrid_player_gfx,
                                PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2) - PRINCESS_HEIGHT_FIXUP,
                                0, PRINCESS_FRAME_WIDTH, PRINCESS_FRAME_HEIGHT);
                    break;

                    case PUZGRID_ANIM_LEFT:
                            COMMON_draw_sprite(puzgrid_player_gfx,
                                PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX) - (puzgrid_animation_clock * 2) ,
                                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX) - PRINCESS_HEIGHT_FIXUP,
                                0, PRINCESS_FRAME_WIDTH, PRINCESS_FRAME_HEIGHT);
                    break;
                }
            }
        }
    }

    glDisable(GL_BLEND);
}

/****************************************************************************************/
/*!
 * @brief Loads a game level.
 *
 * @note The levels are 24x26 PCX image files. Rows 0-23 contain the shape of the
 * walls, the spawn point, the exit position.  Row 24 contains how many monsters can be
 * active on a given level at once, whilst row 25 is the minimum amount of brick-spawning
 * turns to wait before spawning a new monster and dropping them onto the level.
 */
void puzgrid_load_map(const char *filename)
{
    BITMAP *tmp = NULL;
    int x_index, y_index;

    if(exists(filename))
    {
        tmp = al_load_bitmap(filename, NULL);

        // read actual map
        for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
        {
            for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
            {
                // wall
                if (getpixel(tmp, x_index, y_index) == makecol(0,0,0))
                    puzgrid_walls[y_index][x_index] = 1;
                // spawn point
                else if (getpixel(tmp, x_index, y_index) == makecol(255,0,0))
                    puzgrid_walls[y_index][x_index] = PUZGRID_PLAYERSTART;
                // exit
                else if (getpixel(tmp, x_index, y_index) == makecol(0,255,0))
                    puzgrid_walls[y_index][x_index] = PUZGRID_EXIT;
                // empty
                else
                    puzgrid_walls[y_index][x_index] = PUZGRID_EMPTY;
            }
        }

        // read monster data
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if (getpixel(tmp, x_index, PUZGRID_SIZE)     != makecol(255, 255, 255))
                puzgrid_max_monsters = 0;

            if (getpixel(tmp, x_index, PUZGRID_SIZE + 1) != makecol(255, 255, 255))
                puzgrid_turns_between_monsters = 0;
        }

        destroy_bitmap(tmp);
    }
    else // map file wasn't found on disk, load some defaults.
    {
        for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
            for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
                puzgrid_walls[y_index][x_index] = 0;

        puzgrid_walls[1][1] = PUZGRID_EXIT;
        puzgrid_walls[5][5] = PUZGRID_PLAYERSTART;
        puzgrid_max_monsters = 0;
        puzgrid_turns_between_monsters = 0;
    }
}

/**************************************************************************************************/

void puzgrid_randomize_puzzle(void)
{
   int x_index, y_index;
   int gem_colour;

   // fill first two rows
   for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
   {
      puzgrid_gems[0][x_index]   = rand() % puzgrid_difficulty;
      puzgrid_gems[1][x_index]   = rand() % puzgrid_difficulty;

      // force adjacent gems to not match
      if(x_index > 0)
      {
         if(puzgrid_gems[0][x_index] == puzgrid_gems[0][x_index - 1])
         {
            puzgrid_gems[0][x_index] = (puzgrid_gems[0][x_index] + 1) % puzgrid_difficulty;
         }

         if(puzgrid_gems[1][x_index] == puzgrid_gems[0][x_index - 1])
         {
            puzgrid_gems[1][x_index] = (puzgrid_gems[0][x_index] + 1) % puzgrid_difficulty;
         }
      }
   }

   // fill in the rest of the rows. in addition to not matching
   // horizontally, we need to make sure there are no vertically -
   // matched sets of three.
   for(y_index = 2; y_index < PUZGRID_SIZE; y_index++)
   {
      for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
      {
         // choose initial colour...
         puzgrid_gems[y_index][x_index] = rand() % puzgrid_difficulty;

         // ...but is it random enough (vertical)?
         if((puzgrid_gems[y_index][x_index] == puzgrid_gems[y_index - 2][x_index]) &&
            (puzgrid_gems[y_index][x_index] == puzgrid_gems[y_index - 1][x_index]))
         {
            // no - rechoose.
            puzgrid_gems[y_index][x_index] = (puzgrid_gems[y_index][x_index] + 1) % puzgrid_difficulty;
         }

         // ...and is it random enough (horizontal)?
         if(x_index > 0)
         {
            if(puzgrid_gems[y_index][x_index] == puzgrid_gems[y_index][x_index - 1])
               puzgrid_gems[y_index][x_index] = (puzgrid_gems[y_index][x_index] + 1) % puzgrid_difficulty;
         }
      }
   }

   // spawn the player
   for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
   {
      for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
      {
         if(puzgrid_walls[y_index][x_index] == PUZGRID_PLAYERSTART)
         {
             puzgrid_gems[y_index][x_index] = PUZGRID_PLAYER;
         }
      }
   }
}

/*******************************************************************************************/
/*!
 * @brief Checks whether any three-in-a-row or three-in-a-column sets of the same
 * colour have been found and returns either the number of triples found or zero (FALSE).
 */
int puzgrid_check_for_matches(void)
{
    if(puzgrid_animation_clock < 0) return 0;

    puzgrid_num_colours_matched = 0;
    puzgrid_multi_directions_matched = FALSE;

    int retval = 0;
    int got_match_down = FALSE;
    int got_match_across = FALSE;
    int is_l_or_cross = FALSE;
    int is_square = FALSE;
    int num_colours = 0;
    int x_index, y_index;
    int prev_match_colour = 0;
    int num_gems_matched = 0;

    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if ((puzgrid_walls[y_index][x_index] != PUZGRID_WALL) &&
                (puzgrid_walls[y_index][x_index] != PUZGRID_EXIT))
            {
                int colour_to_match = puzgrid_gems[y_index][x_index];

                // check down
                if(y_index < PUZGRID_SIZE - 2)
                {
                    if ((puzgrid_gems[y_index + 1][x_index] == colour_to_match) &&
                        (puzgrid_gems[y_index + 2][x_index] == colour_to_match) &&
                        (puzgrid_walls[y_index + 1][x_index] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index + 2][x_index] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index + 1][x_index] != PUZGRID_EXIT) &&
                        (puzgrid_walls[y_index + 2][x_index] != PUZGRID_EXIT))
                    {
                        // vertical match found
                        retval++;
                        puzgrid_remove[y_index][x_index]      = 1;
                        puzgrid_remove[y_index + 1][x_index]  = 1;
                        puzgrid_remove[y_index + 2][x_index]  = 1;
                        got_match_down = TRUE;

                        // did we get matches in more than one colour?
                        if (prev_match_colour != colour_to_match)
                        {
                            // yes
                            num_colours++;
                        }

                        if (num_colours == 1)
                        {
                            // have we matched more than 3 gems of the same colour?
                            if (num_gems_matched < 3)
                                num_gems_matched = 3;
                            else
                                num_gems_matched++;
                        }

                        prev_match_colour = colour_to_match;
                    }
                }

                // check across
                if(x_index < PUZGRID_SIZE - 2)
                {
                    if ((puzgrid_gems[y_index][x_index + 1] == colour_to_match) &&
                        (puzgrid_gems[y_index][x_index + 2] == colour_to_match) &&
                        (puzgrid_walls[y_index][x_index + 1] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index][x_index + 2] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index][x_index + 1] != PUZGRID_EXIT) &&
                        (puzgrid_walls[y_index][x_index + 2] != PUZGRID_EXIT))
                    {
                        // horizontal match found
                        retval++;
                        puzgrid_remove[y_index][x_index]      = 1;
                        puzgrid_remove[y_index][x_index + 1]  = 1;
                        puzgrid_remove[y_index][x_index + 2]  = 1;
                        got_match_across = TRUE;

                        // did we get matches in more than one colour?
                        if (prev_match_colour != colour_to_match)
                        {
                            // yes
                            num_colours++;
                        }

                        if (num_colours == 1)
                        {
                            // have we matched more than 3 gems of the same colour?
                            if (num_gems_matched < 3)
                                num_gems_matched = 3;
                            else
                                num_gems_matched++;
                        }

                        prev_match_colour = colour_to_match;
                    }
                }

                // check square
                if ((x_index < PUZGRID_SIZE - 1) && (y_index < PUZGRID_SIZE - 1))
                {
                    int colour_to_match = puzgrid_gems[y_index][x_index];

                    if ((puzgrid_gems[y_index][x_index + 1] == colour_to_match) &&
                        (puzgrid_walls[y_index][x_index + 1] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index][x_index + 1] != PUZGRID_EXIT) &&
                        (puzgrid_gems[y_index + 1][x_index] == colour_to_match) &&
                        (puzgrid_walls[y_index + 1][x_index] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index + 1][x_index] != PUZGRID_EXIT) &&
                        (puzgrid_gems[y_index + 1][x_index + 1] == colour_to_match) &&
                        (puzgrid_walls[y_index + 1][x_index + 1] != PUZGRID_WALL) &&
                        (puzgrid_walls[y_index + 1][x_index + 1] != PUZGRID_EXIT))
                    {
                        // square found
                        retval++;
                        puzgrid_remove[y_index][x_index] = 1;
                        puzgrid_remove[y_index+1][x_index] = 1;
                        puzgrid_remove[y_index+1][x_index+1] = 1;
                        puzgrid_remove[y_index][x_index+1] = 1;
                        is_square = TRUE;
                    }
                }

                if (got_match_down && got_match_across)
                    is_l_or_cross = TRUE;
            }
        }
    }

    if (retval > 0) puzgrid_combo_length++;

    puzgrid_num_colours_matched = num_colours;
    puzgrid_multi_directions_matched =  is_l_or_cross;
    puzgrid_square_matched = is_square;
    puzgrid_four_or_more_matched = ((num_gems_matched >= 4) && (!puzgrid_multi_directions_matched));

    return retval;
}

/**************************************************************************************************/
/*!
 * @brief Has the deliberate side effect of flushing the remove array and spawning particle
 *  explosions.
 */
int puzgrid_remove_matched_gems(void)
{
    if(puzgrid_animation_clock < 0) return 0;

    int did_remove = FALSE;

    int num_removed = 0;

    int x_index, y_index, part_index;

    for(y_index = 0; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if(puzgrid_remove[y_index][x_index])
            {
                did_remove = TRUE;
                num_removed++;

                int particle_colour = puzgrid_gems[y_index][x_index];
                puzgrid_gems[y_index][x_index] = -1;    // marked as having no gem at the moment
                puzgrid_remove[y_index][x_index] = 0;   // this slot in the remove grid has been flushed

                // particle explosion
                for(part_index = 0; part_index < 5; part_index++)
                {
                    PARTICLE_spawn(PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),
                        HEART_SPEED, HEART_SPEED, PARTICLE_HEART, HEART_INTENSITY, particle_colour);
                }

                for(part_index = 0; part_index < 3; part_index++)
                {
                    PARTICLE_spawn(PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                        PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),
                        HEART_SPEED, HEART_SPEED, PARTICLE_SPARK, HEART_INTENSITY, particle_colour);
                }
            }
        }
    }

    if(did_remove)
    {
        /// should be in an array :o(
        if(puzgrid_combo_length == 1) play_sample(puzgrid_match1_smp, 255, 128, 1000, 0);
        if(puzgrid_combo_length == 2) play_sample(puzgrid_match2_smp, 255, 128, 1000, 0);
        if(puzgrid_combo_length == 3) play_sample(puzgrid_match3_smp, 255, 128, 1000, 0);
        if(puzgrid_combo_length == 4) play_sample(puzgrid_match4_smp, 255, 128, 1000, 0);
        if(puzgrid_combo_length >= 5) play_sample(puzgrid_match5_smp, 255, 128, 1000, 0);
    }

    return num_removed;
}

/**************************************************************************************************/
int puzgrid_collapse_gems(void)
{
    if(puzgrid_animation_clock < 0) return TRUE;

    int retval = FALSE;
    int x_index, y_index, part_index;
    static int more_randomness;

    for(y_index = PUZGRID_SIZE - 2; y_index >= 0; y_index--)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if((puzgrid_walls[y_index][x_index] != PUZGRID_WALL) && (puzgrid_walls[y_index][x_index] != PUZGRID_EXIT))
            {
                // check for an empty space
                if((puzgrid_gems[y_index + 1][x_index] == -1) && (puzgrid_walls[y_index + 1][x_index] != PUZGRID_EXIT) && (puzgrid_walls[y_index + 1][x_index] != PUZGRID_WALL))
                {
                    // push this gem down
                    puzgrid_gems[y_index + 1][x_index] = puzgrid_gems[y_index][x_index];
                    puzgrid_gems[y_index][x_index] = -1;
                    puzgrid_animations[y_index][x_index] = PUZGRID_ANIM_DOWN;
                    retval = TRUE;
                }

                // case where something fell onto a pile but still needs to animate
                if((puzgrid_walls[y_index][x_index] == 0) &&
                    (puzgrid_gems[y_index][x_index] == -1))
                {
                    puzgrid_animations[y_index][x_index] = PUZGRID_ANIM_DOWN;
                }
            }
        }
    }

    // spawn new gems to fill empty spaces as needed
    // handle the top row
    for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
    {
        if(puzgrid_gems[0][x_index] == -1)
        {
            puzgrid_gems[0][x_index] = rand() % puzgrid_difficulty;

            if((x_index > 0) && (puzgrid_gems[0][x_index] == puzgrid_gems[0][x_index - 1]))
            {
                puzgrid_gems[0][x_index] = (puzgrid_gems[0][x_index] + (more_randomness / 3) + 1) % puzgrid_difficulty;
                more_randomness++;

            }
            // particle explosion
            for(part_index = 0; part_index < 4; part_index++)
            {
                PARTICLE_spawn(PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),
                HEART_SPEED / 3.0, HEART_SPEED / 3.0, PARTICLE_SPARK, HEART_INTENSITY,
                puzgrid_gems[0][x_index]);
            }
        }
    }

    // handle squares that have walls directly above them
    for(y_index = 1; y_index < PUZGRID_SIZE; y_index++)
    {
        for(x_index = 0; x_index < PUZGRID_SIZE; x_index++)
        {
            if((puzgrid_gems[y_index][x_index] == -1) && (puzgrid_walls[y_index - 1][x_index] != 0))
            {
                puzgrid_gems[y_index][x_index] = rand() % puzgrid_difficulty;

                // particle explosion
                for(part_index = 0; part_index < 4; part_index++)
                {
                    PARTICLE_spawn(PUZGRID_GEM_HPOS + (x_index * PUZGRID_TILE_SIZE_IN_PX),
                    PUZGRID_GEM_VPOS + (y_index * PUZGRID_TILE_SIZE_IN_PX),
                    HEART_SPEED / 3.0, HEART_SPEED / 3.0, PARTICLE_SPARK, HEART_INTENSITY / 1.25,
                    puzgrid_gems[y_index][x_index]);
                }
            }
        }
    }

    if (retval) puzgrid_animation_clock = PUZGRID_ANIM_CLOCK_START_VAL;
    return retval;
}


