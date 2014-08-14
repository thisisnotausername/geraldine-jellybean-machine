#include    "gameplay.h"

#define     GAMEPLAY_PUZZLE_SLIDE_IN_START      -512
#define     GAMEPLAY_PUZZLE_SLIDE_IN_END        (int)(80 * (common_aspect_ratio / ((float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT)))
#define     GAMEPLAY_PUZZLE_SLIDE_IN_STEP       8
#define     GAMEPLAY_PUZZLE_YVAL                48

#define     GAMEPLAY_PHASE_SLIDE_IN             1
#define     GAMEPLAY_PHASE_PLAYING              2
#define     GAMEPLAY_PHASE_LOST                 3
#define     GAMEPLAY_PHASE_WON                  4
#define     GAMEPLAY_PHASE_SLIDE_OUT            5
#define     GAMEPLAY_PHASE_NEXT_LEVEL           6
#define     GAMEPLAY_PHASE_QUITTING             7

#define     ROUNDS_TILL_BGM_SWITCH              3
#define     GAMEPLAY_MAX_BGM                    6

#define     GAMEPLAY_OUTCOME_WON                1
#define     GAMEPLAY_OUTCOME_LOST               -1

#define     LEVEL_DISPLAY_SPACING               -4
#define     LEVEL_DISPLAY_SIZE                  18
#define     LEVEL_DISPLAY_X                     32
#define     LEVEL_DISPLAY_Y                     DISPLAY_HEIGHT - (LEVEL_DISPLAY_SIZE * 2)


#define     MAX_LEVELS      10
int gameplay_curr_level = 1;
int gameplay_prev_outcome;

int gameplay_phase;
int gameplay_puzzle_slide_val;

int gameplay_left_clicked = 0;
int gameplay_right_clicked = 0;

int gameplay_rounds_till_bgm_switch;
int gameplay_next_bgm;

void gameplay_handle_playing(void);
void gameplay_handle_lost(void);
void gameplay_handle_won(void);
void gameplay_handle_slide_in(void);
void gameplay_handle_slide_out(void);
void gameplay_handle_next_level(void);
void gameplay_get_mouse_state_debounced(void);

/**************************************************************************************************/
/*!
 * @brief This should probably go into COMMON...
 */

void gameplay_get_mouse_state_debounced(void)
{
    if(mouse_b & 1)
        gameplay_left_clicked++;
    else
        gameplay_left_clicked = 0;

    if(mouse_b & 2)
        gameplay_right_clicked++;
    else
        gameplay_right_clicked = 0;
}

/**************************************************************************************************/
/*!
 * @brief Handle the next tick for when it's time to load the next level; should be considered
 * internal to the gameplay module and not called from elsewhere.
 */
void gameplay_handle_next_level(void)
{
    char filename_buffer[HOME_PATH_LENGTH];

    if (gameplay_prev_outcome == GAMEPLAY_OUTCOME_WON)
    {
        gameplay_curr_level++;
        unlockables[puzgrid_get_difficulty_level()] = gameplay_curr_level;
        UNLOCKED_save();
    }

    //////////////////////////////////////////////
    if (gameplay_curr_level > MAX_LEVELS) { gameplay_phase = GAMEPLAY_PHASE_QUITTING; return; }
    //////////////////////////////////////////////

    // load next map
    snprintf(filename_buffer, HOME_PATH_LENGTH - 1, "./lvl/%02d.pcx", gameplay_curr_level);
    puzgrid_load_map(filename_buffer);
    puzgrid_randomize_puzzle();

    GMCLK_reset(300);
    gameplay_phase = GAMEPLAY_PHASE_SLIDE_IN;
}

/**************************************************************************************************/
/*!
 * @brief Handle the next tick after the player has completed a stage; should be considered
 * internal to the gameplay module and not called from elsewhere.
 */
void gameplay_handle_won(void)
{
    RNDEND_tick();
    GMCLK_tick();   // due to a questionable design choice made early on, the round clock has to be
                    // ticked here, or the '+2.0 sec' sprites will freeze in mid-anim when the player
                    // wins, which is undesirable.

    gameplay_get_mouse_state_debounced();

    if ((gameplay_left_clicked == 1) && RNDEND_done)
    {
        MEFX_spawn_shockring(mouse_x, mouse_y);
        gameplay_phase = GAMEPLAY_PHASE_SLIDE_OUT;
        RNDEND_reset();

        gameplay_rounds_till_bgm_switch--;

        if (gameplay_rounds_till_bgm_switch == 0)
        {
            gameplay_next_bgm = ((rand() + 1 + COMMON_get_bgm()) % GAMEPLAY_MAX_BGM) + 1;
            gameplay_rounds_till_bgm_switch = ROUNDS_TILL_BGM_SWITCH;
            COMMON_fade_bgm();
        }
    }
    else
    if ((gameplay_right_clicked == 1) && RNDEND_done)
    {
        COMMON_fade_bgm();
        SCRNWIPE_start_wipe_out();
        gameplay_phase = GAMEPLAY_PHASE_QUITTING;
        UNLOCKED_save();
    }
}

/**************************************************************************************************/
/*!
 * @brief Initialize the gameplay module.
 */
void GAMEPLAY_init()
{
    //gameplay_curr_level = 1;
    gameplay_next_bgm = ((rand() + 1 + COMMON_get_bgm()) % GAMEPLAY_MAX_BGM) + 1;
    COMMON_set_bgm(gameplay_next_bgm);

    gameplay_rounds_till_bgm_switch = ROUNDS_TILL_BGM_SWITCH;
    gameplay_phase = GAMEPLAY_PHASE_NEXT_LEVEL;
    gameplay_puzzle_slide_val = GAMEPLAY_PUZZLE_SLIDE_IN_START;
    glClear(GL_COLOR_BUFFER_BIT);
    SCRNWIPE_start_wipe_in();
    RNDSTRT_reset();
    puzgrid_move_reticle(128, 128);
    puzgrid_set_difficulty_level(common_next_state_param);
}

/**************************************************************************************************/

void GAMEPLAY_draw()
{
    char buf[HOME_PATH_LENGTH];

    BKGRND_draw();
    puzgrid_draw(gameplay_puzzle_slide_val, GAMEPLAY_PUZZLE_YVAL);
    puzgrid_draw_reticle();

    if (gameplay_phase == GAMEPLAY_PHASE_PLAYING)
    {
        snprintf(buf, HOME_PATH_LENGTH - 1, "Level %d", gameplay_curr_level);
        glColor4f(1,1,1,1);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        COMMON_glprint(common_gamefont, LEVEL_DISPLAY_X, LEVEL_DISPLAY_Y, 1, LEVEL_DISPLAY_SIZE, LEVEL_DISPLAY_SPACING, buf);
    }

    PARTICLE_draw();
    ANCMT_draw();
    GMCLK_draw();
    MEFX_draw();
    RNDSTRT_paint();
    RNDEND_paint();
    SCRNWIPE_draw();

}

/**************************************************************************************************/

void GAMEPLAY_load()
{
    GMCLK_load();
    RNDSTRT_load_assets();
    RNDEND_load_assets();
    puzgrid_load_assets();
    ANCMT_load();
}

/**************************************************************************************************/

void GAMEPLAY_unload()
{
    puzgrid_unload_assets();
    GMCLK_unload();
    RNDSTRT_unload_assets();
    RNDEND_unload_assets();
    ANCMT_unload();
}

/**************************************************************************************************/

void GAMEPLAY_tick()
{
    if (key[KEY_ESC])
    {
        COMMON_fade_bgm();
        SCRNWIPE_start_wipe_out();
        gameplay_phase = GAMEPLAY_PHASE_QUITTING;
    }

    MEFX_tick(mouse_x, mouse_y);
    puzgrid_animate();
    PARTICLE_tick();
    SCRNWIPE_tick();
    ANCMT_tick();
    BKGRND_tick();

    if (common_bgm_fade_done)
        COMMON_set_bgm(gameplay_next_bgm);

    if (scrnwipe_state == SCRNWIPE_STATE_IDLE)
    {
        RNDSTRT_tick();
    }

    switch (gameplay_phase)
    {
        case GAMEPLAY_PHASE_PLAYING:
            gameplay_handle_playing();
        break;

        case GAMEPLAY_PHASE_SLIDE_IN:
            gameplay_handle_slide_in();
        break;

        case GAMEPLAY_PHASE_LOST: /// TODO
        case GAMEPLAY_PHASE_WON:
            gameplay_handle_won();
        break;

        case GAMEPLAY_PHASE_SLIDE_OUT:
            gameplay_handle_slide_out();
        break;

        case GAMEPLAY_PHASE_NEXT_LEVEL:
            gameplay_handle_next_level();
        break;

        case GAMEPLAY_PHASE_QUITTING:
            if ((scrnwipe_state == SCRNWIPE_STATE_IDLE) &&
                (scrnwipe_direction == SCRNWIPE_STATE_SCROLLING_OUT) &&
                (common_bgm_fade_done))
            {
                common_curr_state_done = TRUE;
                common_next_state = STATE_MENU;
            }
        break;
    }
}

/**************************************************************************************************/

void gameplay_handle_playing(void)
{
    if (!RNDSTRT_done) return;

    static int puzzle_needs_collapsing = 0;

    GMCLK_tick();

    // handle interaction with the puzzle.

    gameplay_get_mouse_state_debounced();

    if((gameplay_left_clicked == 1) || (gameplay_right_clicked == 1))
        MEFX_spawn_shockring(mouse_x, mouse_y);

    puzgrid_move_reticle(mouse_x + PUZGRID_BORDER_WIDTH_PX, mouse_y + PUZGRID_BORDER_WIDTH_PX);

    if (puzgrid_animation_clock == 0)
    {
        if(!puzzle_needs_collapsing)
        {
            // the user has just clicked, but wasn't previously
            // holding, the mouse button.
            if((gameplay_left_clicked == 1) || (gameplay_right_clicked == 1))
            {
                // this looks backwards, but it isn't - left-click
                // now spins the gems left; this is more ergonomic
                puzgrid_rotate_reticled_gems(gameplay_right_clicked);

                // each move costs -0.1 sec
                GMCLK_add(-1);
            }

            // Check whether the player has earned any matches.
            puzzle_needs_collapsing = (puzgrid_check_for_matches());

            if (puzzle_needs_collapsing)
            {
                // reward special matches
                if (puzgrid_num_colours_matched > 1)
                {
                    ANCMT_spawn(ANNOUNCEMENT_2_COLS, 0, 0);
                    GMCLK_add(2);
                }

                if (puzgrid_square_matched)
                {
                    ANCMT_spawn(ANNOUNCEMENT_SQUARE, 0, 0);
                    GMCLK_add(2);
                }

                if (puzgrid_multi_directions_matched)
                {
                    ANCMT_spawn(ANNOUNCEMENT_2_DIRS, 0, 0);
                    GMCLK_add(4);
                }
            }

            if(puzgrid_remove_matched_gems())
            {
                ANCMT_spawn(ANNOUNCEMENT_COMBO, puzgrid_combo_length, 0);
                GMCLK_add(12 + (4 * puzgrid_combo_length));
            }
        }

        // we found one or more matches the last time the user
        // moved - start the sliding-down animations and/or
        // check for more matches.
        if(puzzle_needs_collapsing)
            puzzle_needs_collapsing = puzgrid_collapse_gems();

        // did we make it to the exit?
        if (puzgrid_check_player_at_exit())
        {
            gameplay_prev_outcome = GAMEPLAY_OUTCOME_WON;
            gameplay_phase = GAMEPLAY_PHASE_WON;
            RNDEND_start(RNDEND_WIN);
            GMCLK_should_count_down = FALSE;
            return;
        }

        if (GMCLK_get_time_remaining() == 0)
        {
            gameplay_prev_outcome = GAMEPLAY_OUTCOME_LOST;
            gameplay_phase = GAMEPLAY_PHASE_LOST;
            RNDEND_start(RNDEND_LOSE);
            return;
        }
    }
}

/**************************************************************************************************/

void gameplay_handle_slide_out(void)
{
    if (gameplay_puzzle_slide_val > GAMEPLAY_PUZZLE_SLIDE_IN_START)
        gameplay_puzzle_slide_val -= GAMEPLAY_PUZZLE_SLIDE_IN_STEP;
    else if ((common_bgm_fade_done) || (COMMON_get_bgm() == gameplay_next_bgm))
    {
        gameplay_phase = GAMEPLAY_PHASE_NEXT_LEVEL;
        gameplay_puzzle_slide_val = GAMEPLAY_PUZZLE_SLIDE_IN_START;
    }
}

/**************************************************************************************************/

void gameplay_handle_slide_in(void)
{
    if (gameplay_puzzle_slide_val >= GAMEPLAY_PUZZLE_SLIDE_IN_END)
    {
        gameplay_puzzle_slide_val = GAMEPLAY_PUZZLE_SLIDE_IN_END;
        gameplay_phase = GAMEPLAY_PHASE_PLAYING;
        return;
    }

    if (GAMEPLAY_PUZZLE_SLIDE_IN_START == gameplay_puzzle_slide_val)
    {
        // starting from a blank screen (after the main menu)?
        // be sure to start with a screen transition.
        if (scrnwipe_direction == SCRNWIPE_STATE_SCROLLING_OUT)
        {
            glClear(GL_COLOR_BUFFER_BIT);
            SCRNWIPE_start_wipe_in();
        }

        RNDSTRT_reset();
        puzgrid_move_reticle(128, 128);
    }

    gameplay_puzzle_slide_val += GAMEPLAY_PUZZLE_SLIDE_IN_STEP;
}
