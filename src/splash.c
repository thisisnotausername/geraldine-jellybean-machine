#include    "splash.h"
#define     SPLASH_GFX_PATH "./gfx/splash/splash.tga"
#define     SPLASH_SIZE     512


GLuint  splash_gfx;

/**************************************************************************************************/

void SPLASH_init(void)
{
    DUH_WHERE_AM_I("Public Demo #2");
    DUH_WHERE_AM_I("Build number #%04d at %s %s", __BUILDNUM__, __DATE__, __TIME__);
    DUH_WHERE_AM_I("Will write savegames to %s\n", common_user_home_path);
}

/**************************************************************************************************/

void SPLASH_load(void)
{
    splash_gfx = COMMON_load_texture(SPLASH_GFX_PATH);
}

/**************************************************************************************************/

void SPLASH_unload(void)
{
    glDeleteTextures(1, &splash_gfx);
}

/**************************************************************************************************/

void SPLASH_tick(void)
{
    static int num_ticks = 0;

    SCRNWIPE_tick();

    if (num_ticks == 0)
    {
        SCRNWIPE_start_wipe_in();
        num_ticks++;
    }

    if ((num_ticks == 1) && (scrnwipe_state == SCRNWIPE_STATE_IDLE))
    {
        rest(2750);
        num_ticks++;
    }

    if (num_ticks == 2)
        SCRNWIPE_start_wipe_out();

    if ((num_ticks > 2) && (scrnwipe_state == SCRNWIPE_STATE_IDLE) &&
        (scrnwipe_direction == SCRNWIPE_STATE_SCROLLING_OUT))
    {
        common_curr_state_done = TRUE;
        common_next_state = STATE_MENU;
    }

    if((num_ticks > 1) || ((scrnwipe_state == SCRNWIPE_STATE_IDLE) && (scrnwipe_direction == SCRNWIPE_STATE_SCROLLING_IN)))
        num_ticks++;
}

/**************************************************************************************************/

void SPLASH_draw(void)
{
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3f(1,1,1);
    glDisable(GL_BLEND);
    COMMON_draw_sprite(splash_gfx,
        (common_effective_display_width - SPLASH_SIZE) / 2.0,
        (DISPLAY_HEIGHT - SPLASH_SIZE) / 2.0, 1, SPLASH_SIZE, SPLASH_SIZE);

    SCRNWIPE_draw();
}
