/*******************************************************************************************
*
* Part of 'Jellybean Machine' - see common.h for copyright info.
*
*******************************************************************************************/

#define    NUM_BACKGROUNDS  6
#define    NUM_STARS        112
#define    NUM_SCROLLPLANES 3

#define    STAR_SIZE            23
#define    STAR_PERSP_SHRINK    3

#include "common.h"
#include "backgrounds.h"

typedef struct
{
    int x;
    float y;
    int scrollplane;
} BACKGROUND_STAR;

/*******************************************************************************************/

GLTEXTURE bkgrnd_gfx[NUM_BACKGROUNDS];
GLTEXTURE bkgrnd_star_gfx;
BACKGROUND_STAR bkgrnd_stars[NUM_STARS];

int bkgrnd_prev_image;
int bkgrnd_curr_image;
int bkgrnd_next_image;
int bkgrnd_anim_clock;

float bkgrnd_galaxy_scroll_amt;

/*******************************************************************************************/
/*! @brief Resets the stars in the background.
 */
void BKGRND_reset_stars(void)
{
    int index;

    for(index = 0; index < NUM_STARS; index++)
    {
        bkgrnd_stars[index].x           = rand() % DISPLAY_WIDTH;
        bkgrnd_stars[index].y           = rand() % DISPLAY_HEIGHT;
        bkgrnd_stars[index].scrollplane = (rand() % NUM_SCROLLPLANES) + 1;
    }
}

/*******************************************************************************************/
/*! @brief Scrolls the Hubble 'porn' in the background; should be treated as
 *  private.
 */
void BKGRND_tick_galaxy(void)
{

    bkgrnd_galaxy_scroll_amt += 1.0 / (NUM_SCROLLPLANES + 1);

    if(bkgrnd_galaxy_scroll_amt > common_effective_display_width)
    {
        bkgrnd_galaxy_scroll_amt = 0;
        bkgrnd_prev_image        = bkgrnd_curr_image;
        bkgrnd_curr_image        = bkgrnd_next_image;
    }
}

/*******************************************************************************************/

void BKGRND_tick_stars(void)
{
    int index;

    for(index = 0; index < NUM_STARS; index++)
    {
        bkgrnd_stars[index].y += 1.0 / bkgrnd_stars[index].scrollplane;

        if(bkgrnd_stars[index].y >= DISPLAY_HEIGHT)
        {
            bkgrnd_stars[index].x           = rand() % (int)common_effective_display_width;
            bkgrnd_stars[index].y           = 0 - STAR_SIZE;
            bkgrnd_stars[index].scrollplane = (rand() % NUM_SCROLLPLANES) + 1;
        }
    }
}

/*******************************************************************************************/

void BKGRND_draw_stars(void)
{
    int index;
    int dest_col;
    int cur_star_size;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_COLOR, GL_ONE);

    for(index = 0; index < NUM_STARS; index++)
    {
        dest_col = 255 - (bkgrnd_stars[index].scrollplane * 32);
        glColor3ub(dest_col, dest_col, dest_col);
        cur_star_size = STAR_SIZE - (bkgrnd_stars[index].scrollplane * STAR_PERSP_SHRINK);

        COMMON_draw_sprite(bkgrnd_star_gfx, bkgrnd_stars[index].x, bkgrnd_stars[index].y, 3,
            cur_star_size, cur_star_size);
    }
    glDisable(GL_BLEND);
}

/*******************************************************************************************/

void BKGRND_draw_galaxy(void)
{
    glDisable(GL_BLEND);
    glColor3ub(255, 255, 255);

    COMMON_draw_sprite(bkgrnd_gfx[bkgrnd_curr_image],
        0, bkgrnd_galaxy_scroll_amt - common_effective_display_width, 4,
        common_effective_display_width, common_effective_display_width);

    COMMON_draw_sprite(bkgrnd_gfx[bkgrnd_prev_image], 0, bkgrnd_galaxy_scroll_amt, 4,
        common_effective_display_width, common_effective_display_width);
}

/*******************************************************************************************/

void BKGRND_load(void)
{
    char buffer[1024];
    int index;

    for(index = 0; index < NUM_BACKGROUNDS; index++)
    {
        snprintf(buffer, 1023, "./gfx/background/bgscroll%d.tga", index + 1);

        if(!exists(buffer))
            DUH_WHERE_AM_I("missing asset: %s", buffer);

        bkgrnd_gfx[index] = COMMON_load_texture(buffer);
    }

    bkgrnd_star_gfx = COMMON_load_texture("./gfx/background/star.tga");

    BKGRND_reset_stars();
}

/*******************************************************************************************/

void BKGRND_unload(void)
{
    int index;

    for(index = 0; index < NUM_BACKGROUNDS; index++)
    {
        glDeleteTextures(1, &bkgrnd_gfx[index]);
    }

    glDeleteTextures(1, &bkgrnd_star_gfx);
}

/*******************************************************************************************/

void BKGRND_set_next(int which)
{
    bkgrnd_next_image = which % NUM_BACKGROUNDS;
}

/*******************************************************************************************/

void BKGRND_tick(void)
{
    bkgrnd_anim_clock++;

    BKGRND_tick_galaxy();
    BKGRND_tick_stars();
}

/*******************************************************************************************/

void BKGRND_draw(void)
{
    BKGRND_draw_galaxy();
    BKGRND_draw_stars();
}
