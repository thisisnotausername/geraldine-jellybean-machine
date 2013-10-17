#include "screenwipe.h"

#define     WIPE_IN_GFX_PATH    "./gfx/transition/wipe_in.pcx"
#define     WIPE_OUT_GFX_PATH   "./gfx/transition/wipe_out.pcx"

int volatile scrnwipe_state = SCRNWIPE_STATE_IDLE;
int scrnwipe_direction;

int         scrnwipe_hpos = 0;
GLTEXTURE   scrnwipe_in_gfx = 0;
GLTEXTURE   scrnwipe_out_gfx = 0;

/******************************************************************************/
/*!
 * @brief Load the screen transition assets.
 */
void SCRNWIPE_load(void)
{
    if(!exists(WIPE_IN_GFX_PATH))
        OH_SMEG("missing asset: %s", WIPE_IN_GFX_PATH);

    scrnwipe_in_gfx = COMMON_load_texture(WIPE_IN_GFX_PATH);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if(!exists(WIPE_OUT_GFX_PATH))
        OH_SMEG("missing asset: %s", WIPE_OUT_GFX_PATH);

    scrnwipe_out_gfx = COMMON_load_texture(WIPE_OUT_GFX_PATH);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

/******************************************************************************/
/*!
 * @brief Unload the screen transition assets.
 */
void SCRNWIPE_unload(void)
{
    glDeleteTextures(1, &scrnwipe_in_gfx);
    glDeleteTextures(1, &scrnwipe_out_gfx);
}

/******************************************************************************/
/*!
 * @brief Prepare all variables for transitioning from a black screen to
 * gameplay.
 */
void SCRNWIPE_start_wipe_in(void)
{
    if(scrnwipe_state != SCRNWIPE_STATE_IDLE)
        return;

    scrnwipe_state = SCRNWIPE_STATE_ACTIVE;
    scrnwipe_direction = SCRNWIPE_STATE_SCROLLING_IN;
    scrnwipe_hpos = common_effective_display_width;
}

/******************************************************************************/
/*!
 * @brief Prepare all variables for transitioning from gameplay to a black
 * screen.
 */
void SCRNWIPE_start_wipe_out(void)
{
    if(scrnwipe_state != SCRNWIPE_STATE_IDLE)
        return;

    scrnwipe_state = SCRNWIPE_STATE_ACTIVE;
    scrnwipe_direction = SCRNWIPE_STATE_SCROLLING_OUT;
    scrnwipe_hpos = common_effective_display_width;
}

/******************************************************************************/
/*!
 * @brief Should be called once per frame to animate the transition; will do
 * nothing if a transition isn't currently active.
 */
void SCRNWIPE_tick(void)
{
    if(scrnwipe_state == SCRNWIPE_STATE_IDLE)
        return;

    scrnwipe_hpos -= SCRNWIPE_SLIDE_SPEED;

    if(scrnwipe_hpos <= -common_effective_display_width)
    {
        scrnwipe_state = SCRNWIPE_STATE_IDLE;
    }
}

/******************************************************************************/
/*!
 * @brief Paint the transition over the contents of the display.
 * @note Doesn't restore the blend state.
 */
void SCRNWIPE_draw(void)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_ZERO, GL_ONE_MINUS_SRC_COLOR);

    switch (scrnwipe_direction)
    {
        case SCRNWIPE_STATE_SCROLLING_IN:
            // the wipe effect graphic
            glColor3ub(255, 255, 255);
            COMMON_draw_sprite(scrnwipe_in_gfx, scrnwipe_hpos, 0, -1,
                common_effective_display_width, DISPLAY_HEIGHT);

            if (scrnwipe_hpos > 0)
            {
                // the fully black area of the display
                glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_2D);
                glColor3ub(0, 0, 0);
                COMMON_draw_sprite(scrnwipe_in_gfx, 0, 0, -1 , scrnwipe_hpos, DISPLAY_HEIGHT);
                glEnable(GL_BLEND);
                glEnable(GL_TEXTURE_2D);
            }
        break;

        case SCRNWIPE_STATE_SCROLLING_OUT:
            // the wipe effect graphic
            COMMON_draw_sprite(scrnwipe_out_gfx, scrnwipe_hpos, 0, -1,
                common_effective_display_width, DISPLAY_HEIGHT);

            if(scrnwipe_hpos < 0)
            {
                // the fully black area of the display
                glDisable(GL_BLEND);
                glDisable(GL_TEXTURE_2D);
                glColor3ub(0, 0, 0);
                // the -1 fixes some bizarro flickering that happens due to roundoff error
                COMMON_draw_sprite(scrnwipe_in_gfx, scrnwipe_hpos + common_effective_display_width - 1,
                    0, -1, common_effective_display_width + SCRNWIPE_SLIDE_SPEED, DISPLAY_HEIGHT);
                glEnable(GL_BLEND);
                glEnable(GL_TEXTURE_2D);
            }
        break;
    }
}
