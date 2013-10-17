/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/

#include "announcements.h"

/******************************************************************************************/
/*! @defgroup combo_sprite_stuff
 * @brief This is all the stuff used for creating, animating and drawing the
 * 'combo!!!' sprites.
 * @{
 */
#define TWO_COLS_GFX_PATH                 "./gfx/announcement/2colours.pcx"
#define TWO_DIRS_GFX_PATH                 "./gfx/announcement/2directions.pcx"
#define SQUARE_GFX_PATH                   "./gfx/announcement/square.pcx"
#define COMBO_GFX_PATH                    "./gfx/announcement/%1dcombo.pcx"
#define NUM_COMBO_SPRITES                 32
#define NUM_SPECIAL_SPRITES               8
#define COMBO_WIDTH                       180
#define COMBO_HEIGHT                      90
#define COMBO_HORZ_POS                    common_effective_display_width - COMBO_WIDTH - (int)(64 * common_aspect_ratio)
#define COMBO_HORZ_RAND                   -(rand() % 40)
#define COMBO_VERT_POS                    480
#define COMBO_TIME_TO_LIVE                75
#define COMBO_DRAW_MULTIPLIER             3
#define NUM_COMBO_IMGS                    4

typedef struct
{
    int x;
    int y;
    int time_to_live;
    int combo_length;
} COMBO_SPRITE;

COMBO_SPRITE ancmt_combo_pool[NUM_COMBO_SPRITES];
COMBO_SPRITE ancmt_special_pool[NUM_SPECIAL_SPRITES];

GLTEXTURE ancmt_combo_gfx[NUM_COMBO_IMGS];
GLTEXTURE ancmt_2cols_gfx;
GLTEXTURE ancmt_2dirs_gfx;
GLTEXTURE ancmt_square_gfx;

void ANCMT_load_combo(void);
void ANCMT_unload_combo(void);
void ANCMT_spawn_combo(int combo_length);
void ANCMT_spawn_special(int which);
void ANCMT_tick_combo(void);
void ANCMT_draw_combo(void);
void ANCMT_reset_combo(void);

/*! @} */
/******************************************************************************************/
/*! @defgroup minus_time_sprite_stuff
 * @brief This is all the stuff used for creating, animating and drawing the
 * '-0.1 sec' sprites.
 * @{
 */
#define MINUS_TIME_GFX_PATH                     "./gfx/announcement/minus_one.pcx"
#define NUM_TIME_LOST_SPRITES                   64
#define TIME_LOST_WIDTH                         128
#define TIME_LOST_HEIGHT                        64
#define ANCMT_TLOST_HORZ_POS                    common_effective_display_width - TIME_LOST_WIDTH - 16
#define ANCMT_TLOST_HORZ_RAND                   -(rand() % 40)
#define ANCMT_TLOST_VERT_POS                    512
#define ANCMT_TLOST_TIME_TO_LIVE                30
#define ANCMT_TLOST_DRAW_MULTIPLIER             3

typedef struct
{
    int x;
    int y;
    int time_to_live;
} TIME_LOST_SPRITE;

TIME_LOST_SPRITE ancmt_time_lost_pool[NUM_TIME_LOST_SPRITES];

GLTEXTURE ancmt_minus_time_gfx;

void ANCMT_load_time_lost(void);
void ANCMT_unload_time_lost(void);
void ANCMT_spawn_time_lost(void);
void ANCMT_tick_time_lost(void);
void ANCMT_draw_time_lost(void);
void ANCMT_reset_time_lost(void);

/*! @} */
/******************************************************************************************/
/*! @brief Load the 'combo!' images. Should be considered private to this module.
 */
void ANCMT_load_combo(void)
{
    int index;
    char buf[1024];

    for (index = 0; index < NUM_COMBO_IMGS; index++)
    {
        snprintf(buf, 1023, COMBO_GFX_PATH, index + 2);
        ancmt_combo_gfx[index] = COMMON_load_texture(buf);
    }

    ancmt_2cols_gfx     = COMMON_load_texture(TWO_COLS_GFX_PATH);
    ancmt_2dirs_gfx     = COMMON_load_texture(TWO_DIRS_GFX_PATH);
    ancmt_square_gfx    = COMMON_load_texture(SQUARE_GFX_PATH);
}

/******************************************************************************************/
/*! @brief Unload the combo images. Should be considered private to this module.
 */
void ANCMT_unload_combo(void)
{
    int index;

    for(index = 0; index < NUM_COMBO_IMGS; index++)
        glDeleteTextures(1, &ancmt_combo_gfx[index]);

    glDeleteTextures(1, &ancmt_2cols_gfx);
    glDeleteTextures(1, &ancmt_2dirs_gfx);
    glDeleteTextures(1, &ancmt_square_gfx);
}

/******************************************************************************************/
/*! @brief Load the time-lost image. Should be considered private to this module.
 */
void ANCMT_load_time_lost(void)
{
    if (!exists(MINUS_TIME_GFX_PATH))
        OH_SMEG("missing image: %s", MINUS_TIME_GFX_PATH);
    ancmt_minus_time_gfx = COMMON_load_texture(MINUS_TIME_GFX_PATH);
}

/******************************************************************************************/
/*! @brief Unload the time-lost image. Should be considered private to this module.
 */
void ANCMT_unload_time_lost(void)
{
    glDeleteTextures(1, &ancmt_minus_time_gfx);
}

/******************************************************************************************/
/*! @brief Launches a '-0.1 sec' sprite. Should be considered private to this module.
 */
void ANCMT_spawn_time_lost(void)
{
    static int index;

    // populate this sprite with its default data
    ancmt_time_lost_pool[index].x            = ANCMT_TLOST_HORZ_POS + ANCMT_TLOST_HORZ_RAND;
    ancmt_time_lost_pool[index].y            = ANCMT_TLOST_VERT_POS;
    ancmt_time_lost_pool[index].time_to_live = ANCMT_TLOST_TIME_TO_LIVE;

    // remember where we are in the pool
    index++;
    if(index >= NUM_TIME_LOST_SPRITES)
        index = 0;
}

/******************************************************************************************/
/*! @brief Launches a 'combo!' sprite. Should be considered private to this module.
 */
void ANCMT_spawn_combo(int combo_length)
{
    static int index;

    if (combo_length < 2) return;           // not a combo
    if (combo_length > 5) combo_length = 5; // no rating higher than five stars

    // populate this sprite with its default data
    ancmt_combo_pool[index].x            = COMBO_HORZ_POS + COMBO_HORZ_RAND;
    ancmt_combo_pool[index].y            = COMBO_VERT_POS;
    ancmt_combo_pool[index].time_to_live = COMBO_TIME_TO_LIVE;
    ancmt_combo_pool[index].combo_length = combo_length;

    int tmp;
    int num_partcls = 16 + (8 * combo_length);
    for(tmp = 0; tmp < num_partcls ; tmp++)
        PARTICLE_spawn(ancmt_combo_pool[index].x + (rand() % COMBO_WIDTH),
            COMBO_VERT_POS,
            ((rand() % 32) / 4.0) - 3.5, ((rand() % 32) / 4.0) - 3.5,
            PARTICLE_SPARK, 224, rand() % NUM_PARTICLE_COLOURS);

    // remember where we are in the pool
    index++;
    if(index >= NUM_COMBO_SPRITES)
        index = 0;
}

/******************************************************************************************/
/*! @brief Document me, please.
 */
void ANCMT_spawn_special(int which)
{
    static int index;

    // populate this sprite with its default data
    ancmt_special_pool[index].x            = COMBO_HORZ_POS + COMBO_HORZ_RAND;
    ancmt_special_pool[index].y            = COMBO_VERT_POS;
    ancmt_special_pool[index].time_to_live = COMBO_TIME_TO_LIVE;
    ancmt_special_pool[index].combo_length = which;

    // add a teensy delay
    switch (which)
    {
        case ANNOUNCEMENT_2_COLS:
            ancmt_special_pool[index].time_to_live += 16;
        break;

        case ANNOUNCEMENT_2_DIRS:
            ancmt_special_pool[index].time_to_live += 32;
        break;

        case ANNOUNCEMENT_SQUARE:
            ancmt_special_pool[index].time_to_live += 48;
        break;
    }

    // remember where we are in the pool
    index++;
    if(index >= NUM_SPECIAL_SPRITES)
        index = 0;
}

/******************************************************************************************/
/*! @brief Advances the animations on all the time-lost sprites. Should be considered
 *  private to this module.
 */
void ANCMT_tick_time_lost(void)
{
    int index;

    for(index = 0; index < NUM_TIME_LOST_SPRITES; index++)
    {
        // if this sprite is alive, move it up slightly and
        // fade it out
        if(ancmt_time_lost_pool[index].time_to_live > 0)
        {
            ancmt_time_lost_pool[index].y--;
            ancmt_time_lost_pool[index].time_to_live--;
        }
    }
}

/******************************************************************************************/
/*! @brief Resets all the time-lost sprites. Should be considered
 *  private to this module.
 */
void ANCMT_reset_time_lost(void)
{
    int index;

    for(index = 0; index < NUM_TIME_LOST_SPRITES; index++)
    {
        ancmt_time_lost_pool[index].time_to_live = 0;
    }
}

/******************************************************************************************/
/*! @brief Resets all the combo sprites. Should be considered private to this module.
 */
void ANCMT_reset_combo(void)
{
    int index;

    for(index = 0; index < NUM_TIME_LOST_SPRITES; index++)
    {
        ancmt_time_lost_pool[index].time_to_live = 0;
    }
}

/******************************************************************************************/
/*! @brief Advances the animations on all the combo sprites. Should be considered
 *  private to this module.
 */
void ANCMT_tick_combo(void)
{
    int index;

    for (index = 0; index < NUM_COMBO_SPRITES; index++)
    {
        // if this sprite is alive, move it up slightly and
        // fade it out
        if (ancmt_combo_pool[index].time_to_live > 0)
        {
            ancmt_combo_pool[index].y--;
            ancmt_combo_pool[index].time_to_live--;
        }
    }

    for (index = 0; index < NUM_SPECIAL_SPRITES; index++)
    {
        // if this sprite is alive, move it up slightly and
        // fade it out
        if (ancmt_special_pool[index].time_to_live > 0)
        {
            if (ancmt_special_pool[index].time_to_live < COMBO_TIME_TO_LIVE)
                ancmt_special_pool[index].y--;
            ancmt_special_pool[index].time_to_live--;
        }
    }
}

/******************************************************************************************/
/*! @brief Draw all the time-lost sprites. Should be considered private to this module.
 */
void ANCMT_draw_time_lost(void)
{
    int index;

    // draw all '-0.1 sec' sprites with additive blending,
    // fading them as they rise
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);

    for(index = 0; index < NUM_TIME_LOST_SPRITES; index++)
    {
        if(ancmt_time_lost_pool[index].time_to_live > 0)
        {
            float intensity =
                ancmt_time_lost_pool[index].time_to_live / (float)ANCMT_TLOST_TIME_TO_LIVE;

            glColor3f(intensity, intensity, intensity);

            COMMON_draw_sprite(ancmt_minus_time_gfx,
                ancmt_time_lost_pool[index].x, ancmt_time_lost_pool[index].y, -1,
                TIME_LOST_WIDTH, TIME_LOST_HEIGHT);
        }
    }
}

/******************************************************************************************/
/*! @brief Draw all the time-lost sprites. Should be considered private to this module.
 */
void ANCMT_draw_combo(void)
{
    int index;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);

    for (index = 0; index < NUM_COMBO_SPRITES; index++)
    {
        if (ancmt_combo_pool[index].time_to_live > 0)
        {
            float intensity =
                ancmt_combo_pool[index].time_to_live / (float)COMBO_TIME_TO_LIVE;

            glColor3f(intensity, intensity, intensity);

            COMMON_draw_sprite(ancmt_combo_gfx[ancmt_combo_pool[index].combo_length - 2],
                ancmt_combo_pool[index].x, ancmt_combo_pool[index].y, -1,
                COMBO_WIDTH, COMBO_HEIGHT);
        }
    }

    for (index = 0; index < NUM_SPECIAL_SPRITES; index++)
    {
        if ((ancmt_special_pool[index].time_to_live > 0) &&
            (ancmt_special_pool[index].time_to_live <= COMBO_TIME_TO_LIVE))
        {
            if (ancmt_special_pool[index].time_to_live == COMBO_TIME_TO_LIVE)
            {
                int tmp;
                int num_partcls = 12;
                for(tmp = 0; tmp < num_partcls ; tmp++)
                    PARTICLE_spawn(ancmt_special_pool[index].x + (rand() % COMBO_WIDTH),
                        COMBO_VERT_POS,
                        ((rand() % 32) / 4.0) - 3.5, ((rand() % 32) / 4.0) - 3.5,
                        PARTICLE_SPARK, 224, rand() % NUM_PARTICLE_COLOURS);
            }

            float intensity =
                ancmt_special_pool[index].time_to_live / (float)COMBO_TIME_TO_LIVE;

            glColor3f(intensity, intensity, intensity);

            switch (ancmt_special_pool[index].combo_length)
            {
                case ANNOUNCEMENT_2_COLS:
                    COMMON_draw_sprite(ancmt_2cols_gfx,
                        ancmt_special_pool[index].x, ancmt_special_pool[index].y, -1,
                        COMBO_WIDTH, COMBO_HEIGHT);
                break;

                case ANNOUNCEMENT_2_DIRS:
                    COMMON_draw_sprite(ancmt_2dirs_gfx,
                        ancmt_special_pool[index].x, ancmt_special_pool[index].y, -1,
                        COMBO_WIDTH, COMBO_HEIGHT);
                break;

                case ANNOUNCEMENT_SQUARE:
                    COMMON_draw_sprite(ancmt_square_gfx,
                        ancmt_special_pool[index].x, ancmt_special_pool[index].y, -1,
                        COMBO_WIDTH, COMBO_HEIGHT);
                break;
            }
        }
    }
}

/**************************************************************************************************/

int ancmt_animation_clock = 0; // ANNOUNCEMENT_ANIM_CLOCK_IDLE;
int ancmt_state;

/**************************************************************************************************/

void ANCMT_load(void)
{
    ANCMT_load_combo();
    ANCMT_load_time_lost();
}

/**************************************************************************************************/

void ANCMT_unload(void)
{
    ANCMT_unload_time_lost();
    ANCMT_unload_combo();
}

/**************************************************************************************************/

void ANCMT_spawn(int which, int combo_length, int added_time)
{
    switch(which)
    {
        case ANNOUNCEMENT_MINUS_TENTH_SEC:
            ANCMT_spawn_time_lost();
        break;

        case ANNOUNCEMENT_COMBO:
            ANCMT_spawn_combo(combo_length);
        break;

        case ANNOUNCEMENT_2_COLS:
        case ANNOUNCEMENT_2_DIRS:
        case ANNOUNCEMENT_SQUARE:
            ANCMT_spawn_special(which);
        break;
    }
}

/**************************************************************************************************/

void ANCMT_tick(void)
{
    ANCMT_tick_time_lost();
    ANCMT_tick_combo();
}

/**************************************************************************************************/

void ANCMT_draw(void)
{
    ANCMT_draw_time_lost();
    ANCMT_draw_combo();
}
