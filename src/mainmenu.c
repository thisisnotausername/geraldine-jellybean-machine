#include "common.h"
#include "mainmenu.h"
#include "backgrounds.h"
#include "gameplay.h"
#include "unlocks.h"

#define MENU_ITEM_HEIGHT    36
#define MENU_FONT_SIZE      32

#define LOGOTYPE_WIDTH  360
#define LOGOTYPE_HEIGHT 180
#define LOGOTYPE_HORZ   ((common_effective_display_width - LOGOTYPE_WIDTH) / 2.0)
#define LOGOTYPE_VERT   (40)
#define LOGOTYPE_PATH   "./gfx/mainmenu/logo.tga"
GLTEXTURE mmenu_logotype;

#define HIGHLIGHT_WIDTH  512
#define HIGHLIGHT_HEIGHT 48
#define HIGHLIGHT_PATH   "./gfx/mainmenu/highlight.pcx"
GLTEXTURE mmenu_item_highlight;

#define MMSTATE_CLICK_TO_START      1
#define MMSTATE_UNLOCKED            2
#define MMSTATE_CONFIRM_QUIT        3
#define MMSTATE_MAIN                4
int mmenu_internal_state;

#define MM_CLICK_SOUND_PATH    "./snd/rotate.wav"
ALLEGRO_SAMPLE *mmenu_click;

#define MM_START_SOUND_PATH    "./snd/start.wav"
ALLEGRO_SAMPLE *mmenu_start;

int mmenu_anim_clock;
int mmenu_difficulty = PUZGRID_EASY;
int mmenu_is_level_locked = FALSE;

int mmenu_which_item_has_cursor;
char mmenu_lvl_skip_buf[256]; //! < Used for composing the string that forms the level-skip menu item


/******************************************************************************************/
/*!
 * @brief Determine which, if any, menu item should be drawn with the highlight; this is
 * done by checking whether the mouse cursor is inside the menu item's bounding box.
 *
 * Apologies to whoever finds this; I know it's a trainwreck, and the only thing I can guess
 * about it is that I speed-coded it originally and never came back to fix it.
 */
void MAINMENU_check_highlighted(void)
{

    if ((mouse_x < ((common_effective_display_width - HIGHLIGHT_WIDTH) / 2)) ||
        (mouse_x > ((common_effective_display_width + HIGHLIGHT_WIDTH) / 2)))
    {
        mmenu_which_item_has_cursor = -1;
        return;
    }

    if ((mouse_y > LOGOTYPE_HEIGHT + LOGOTYPE_VERT + MENU_ITEM_HEIGHT) &&
        (mouse_y < LOGOTYPE_HEIGHT + LOGOTYPE_VERT + MENU_ITEM_HEIGHT + MENU_ITEM_HEIGHT))
    {
        mmenu_which_item_has_cursor = 0;
        return;
    }

    if ((mouse_y > LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 2)) &&
        (mouse_y < LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 2) + MENU_ITEM_HEIGHT))
    {
        mmenu_which_item_has_cursor = 1;
        return;
    }

    if ((mouse_y > LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 3)) &&
        (mouse_y < LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 3) + MENU_ITEM_HEIGHT))
    {
        mmenu_which_item_has_cursor = 2;
        return;
    }

    if ((mouse_y > LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 4)) &&
        (mouse_y < LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 4) + MENU_ITEM_HEIGHT))
    {
        mmenu_which_item_has_cursor = 3;
        return;
    }

    mmenu_which_item_has_cursor = -1;
    return;
}

/******************************************************************************************/
/*!
 * @brief Set up everything needed to begin displaying and interacting with the main menu.
 */
void MAINMENU_init()
{
    // populate the table of what's unlocked from the savegame
    UNLOCKED_load();

    // do this here, so it happens at least once, otherwise, we'll get a blank space (oops)
    snprintf(mmenu_lvl_skip_buf, 255, "Play Level %d", gameplay_curr_level);

    BKGRND_set_next(1);
    mmenu_internal_state = MMSTATE_CLICK_TO_START;
    COMMON_set_bgm(99);
    mmenu_anim_clock = 0;
    SCRNWIPE_start_wipe_in();
}

/******************************************************************************************/
/*!
 * @brief Load all of the main menu's assets.
 */
void MAINMENU_load()
{
    mmenu_logotype          = COMMON_load_texture(LOGOTYPE_PATH);
    mmenu_item_highlight    = COMMON_load_texture(HIGHLIGHT_PATH);
    mmenu_click             = al_load_sample(MM_CLICK_SOUND_PATH);
    mmenu_start             = al_load_sample(MM_START_SOUND_PATH);
}

/******************************************************************************************/
/*!
 * @brief Unload all the main menu's assets and free up memory.
 */
void MAINMENU_unload()
{
    glDeleteTextures(1, &mmenu_logotype       );
    glDeleteTextures(1, &mmenu_item_highlight );
    destroy_sample(mmenu_click);
    destroy_sample(mmenu_start);
}

/******************************************************************************************/
/*!
 * @brief Advance all logic and animations in the main menu screen by one 'tick'.
 */
void MAINMENU_tick()
{
    static int left_clicked = 0;
    static int right_clicked = 0;

    if (key[KEY_ESC])
    {
        common_next_state = STATE_EXIT_APP;
        COMMON_fade_bgm();
        SCRNWIPE_start_wipe_out();
        mmenu_internal_state = MMSTATE_MAIN;
    }

    MEFX_tick(mouse_x, mouse_y);
    PARTICLE_tick();
    SCRNWIPE_tick();
    BKGRND_tick();

    MAINMENU_check_highlighted();

    if (scrnwipe_state != SCRNWIPE_STATE_IDLE)
        return;

    mmenu_anim_clock++;

    if(mouse_b & 1)
        left_clicked++;
    else
        left_clicked = 0;

    if(mouse_b & 2)
        right_clicked++;
    else
        right_clicked = 0;

    if((left_clicked == 1) || (right_clicked == 1))
        MEFX_spawn_shockring(mouse_x, mouse_y);

    switch (mmenu_internal_state)
    {
        case MMSTATE_CLICK_TO_START:
            if((left_clicked == 1) || (right_clicked == 1))
            {
                mmenu_internal_state = MMSTATE_MAIN;
                play_sample(mmenu_start, 255, 128, 1000, 0);
            }
        break;

        case MMSTATE_MAIN:
            if ((scrnwipe_direction == SCRNWIPE_STATE_SCROLLING_OUT) && (common_bgm_fade_done))
            {
                common_curr_state_done = TRUE;
                return;
            }

            if((left_clicked == 1) || (right_clicked == 1))
            {
                play_sample(mmenu_click, 255, 128, 1000, 0);
                switch (mmenu_which_item_has_cursor)
                {
                    case 0:
                        if (!mmenu_is_level_locked)
                        {
                            common_next_state = STATE_GAMEPLAY;
                            common_next_state_param = mmenu_difficulty;
                            COMMON_fade_bgm();
                            SCRNWIPE_start_wipe_out();
                        }
                    break;

                    case 1:

                        mmenu_difficulty++;
                        if (mmenu_difficulty > PUZGRID_HARD)
                            mmenu_difficulty = PUZGRID_EASY;

                        // because difficulty selection affects which levels can be played,
                        // we have to do this here, too...
                        //
                        // Yes, I know it violates the DRY rule. I am sorry.
                        //
                        // is this level unlocked yet?
                        //
                        // NOTE: do _not_ remove or alter the - PUZGRID_EASY; it maps PUZGRID_* difficulties
                        // onto UNLOCKS_* ones.  These can't change without a lot of hassle; it
                        // really isn't worth the level of annoyance to fix it, trust me.
                        if (gameplay_curr_level > unlockables[mmenu_difficulty - PUZGRID_EASY])
                        {
                            // no - user needs to complete previous level successfully first.
                            snprintf(mmenu_lvl_skip_buf, 255, "Level %d Locked - Play %d First", gameplay_curr_level, gameplay_curr_level - 1);
                            mmenu_is_level_locked = TRUE;
                        }
                        else
                        {
                            // yes - available for play
                            snprintf(mmenu_lvl_skip_buf, 255, "Play Level %d", gameplay_curr_level);
                            mmenu_is_level_locked = FALSE;
                        }
                    break;

                    case 2:

                        if (left_clicked == 1)
                            gameplay_curr_level++;
                        if (right_clicked == 1)
                            gameplay_curr_level--;

                        if (gameplay_curr_level < 1)
                            gameplay_curr_level = 1;

                        // NOTE: do _not_ remove or alter the - PUZGRID_EASY; it maps PUZGRID_* difficulties
                        // onto UNLOCKS_* ones.  These can't change without a lot of hassle; it
                        // really isn't worth the level of annoyance to fix it, trust me.
                        //
                        // is this level unlocked yet?
                        if (gameplay_curr_level > unlockables[mmenu_difficulty - PUZGRID_EASY])
                        {
                            // no - user needs to complete previous level successfully first.
                            snprintf(mmenu_lvl_skip_buf, 255, "Level %d Locked - Play %d First", gameplay_curr_level, gameplay_curr_level - 1);
                            mmenu_is_level_locked = TRUE;
                        }
                        else
                        {
                            // yes - available for play
                            snprintf(mmenu_lvl_skip_buf, 255, "Play Level %d", gameplay_curr_level);
                            mmenu_is_level_locked = FALSE;
                        }
                    break;


                    case 3:
                        common_next_state = STATE_EXIT_APP;
                        COMMON_fade_bgm();
                        SCRNWIPE_start_wipe_out();
                    break;
                }
            }
        break;
    }
}

/******************************************************************************************/
/*!
 * @brief Draws the text in the main menu.
 *
 * There are Shlemielisms all over the place in here, but, as they don't seem to affect
 * performance, I'm not inclined to mess with them at the moment.
 */
void MAINMENU_draw()
{
    BKGRND_draw();

    glDisable(GL_BLEND);
    glColor3ub(255, 255, 255);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    COMMON_draw_sprite(mmenu_logotype, LOGOTYPE_HORZ, LOGOTYPE_VERT, 4, LOGOTYPE_WIDTH, LOGOTYPE_HEIGHT);

    switch (mmenu_internal_state)
    {
        case MMSTATE_CLICK_TO_START:
        {
            glColor4f(1, 1, 1, sin(mmenu_anim_clock / 62.3f) / 2.0f + 0.5);
            COMMON_glprint(common_gamefont, (common_effective_display_width - ((MENU_FONT_SIZE - 8) * 21)) / 2.0,
                LOGOTYPE_HEIGHT + LOGOTYPE_VERT + MENU_ITEM_HEIGHT, 3, MENU_FONT_SIZE, -8, "*  Click To Start!  *");
        }
        break;

        case MMSTATE_MAIN:
        {
            if (mmenu_which_item_has_cursor != -1)
            {
                glColor3f(  (sin((mmenu_anim_clock - 30) / 48.1f) / 4.0f) + 0.7,
                            (sin((mmenu_anim_clock + 30) / 57.5f) / 4.0f) + 0.7,
                            (sin((mmenu_anim_clock - 72) / 52.4f) / 4.0f) + 0.7);
                glBlendFunc(GL_SRC_COLOR, GL_ONE);
                COMMON_draw_sprite(mmenu_item_highlight,
                    (common_effective_display_width - (HIGHLIGHT_WIDTH + (sin(mmenu_anim_clock / 36.7) * 48))) / 2.0,
                    LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * (mmenu_which_item_has_cursor + 1) - (HIGHLIGHT_HEIGHT / 4.0f)),
                    4, (HIGHLIGHT_WIDTH + (sin(mmenu_anim_clock / 36.7) * 48)), HIGHLIGHT_HEIGHT);
            }

            glColor4f(1, 1, 1, 1);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen("Play"))) / 2.0,
                LOGOTYPE_HEIGHT + LOGOTYPE_VERT + MENU_ITEM_HEIGHT, 3, MENU_FONT_SIZE, -8, "Play");

            switch (mmenu_difficulty)
            {
                case PUZGRID_EASY:
                    COMMON_glprint(common_gamefont,
                        (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen("Difficulty: Easy"))) / 2.0,
                        LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 2), 3, MENU_FONT_SIZE, -8, "Difficulty: Easy");
                break;

                case PUZGRID_MEDIUM:
                    COMMON_glprint(common_gamefont,
                        (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen("Difficulty: Medium"))) / 2.0,
                        LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 2), 3, MENU_FONT_SIZE, -8, "Difficulty: Medium");
                break;

                case PUZGRID_HARD:
                    COMMON_glprint(common_gamefont,
                        (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen("Difficulty: Hard"))) / 2.0,
                        LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 2), 3, MENU_FONT_SIZE, -8, "Difficulty: Hard");
                break;
            }

            // display this in red if the currently-selected level is locked
            if (mmenu_is_level_locked)
                glColor4f(1, 0, 0, 1);

            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen(mmenu_lvl_skip_buf))) / 2.0,
                LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 3), 3, MENU_FONT_SIZE, -8, mmenu_lvl_skip_buf);

            glColor4f(1, 1, 1, 1);
            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((MENU_FONT_SIZE - 8) * strlen("Exit"))) / 2.0,
                LOGOTYPE_HEIGHT + LOGOTYPE_VERT + (MENU_ITEM_HEIGHT * 4), 3, MENU_FONT_SIZE, -8, "Exit");
        }
    }

    glDisable(GL_BLEND);

    MEFX_draw();
    PARTICLE_draw();
    SCRNWIPE_draw();
}
