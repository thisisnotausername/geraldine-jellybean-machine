#include "common.h"
#include "puzgrid.h"
#include "mouse_effects.h"
#include "particles.h"
#include "screenwipe.h"
#include "announcements.h"
#include "mainmenu.h"
#include "gameclock.h"
#include "gameplay.h"
#include "splash.h"
#include "round_start_anim.h"
#include "round_end_anim.h"

/**********************************************************************************************/

int main(void)
{
    GAMESTATE game_states[3];

    game_states[STATE_SPLASH].tick       = SPLASH_tick;
    game_states[STATE_SPLASH].draw       = SPLASH_draw;
    game_states[STATE_SPLASH].load       = SPLASH_load;
    game_states[STATE_SPLASH].unload     = SPLASH_unload;
    game_states[STATE_SPLASH].init       = SPLASH_init;

    game_states[STATE_MENU].tick         = MAINMENU_tick;
    game_states[STATE_MENU].draw         = MAINMENU_draw;
    game_states[STATE_MENU].load         = MAINMENU_load;
    game_states[STATE_MENU].unload       = MAINMENU_unload;
    game_states[STATE_MENU].init         = MAINMENU_init;

    game_states[STATE_GAMEPLAY].tick     = GAMEPLAY_tick;
    game_states[STATE_GAMEPLAY].draw     = GAMEPLAY_draw;
    game_states[STATE_GAMEPLAY].load     = GAMEPLAY_load;
    game_states[STATE_GAMEPLAY].unload   = GAMEPLAY_unload;
    game_states[STATE_GAMEPLAY].init     = GAMEPLAY_init;

    COMMON_setup(TRUE);

    MEFX_load();
    PARTICLE_load();
    PARTICLE_init();
    SCRNWIPE_load();
    BKGRND_load();

    common_next_state = STATE_SPLASH;

    game_states[common_next_state].load();
    game_states[common_next_state].init();

    common_curr_state = common_next_state;

    while((common_curr_state != STATE_EXIT_APP))
    {
        while(!common_curr_state_done)
        {

// #ifdef _WIN32
            if(common_time_for_logic)
            {
                poll_keyboard();
                poll_mouse();
                COMMON_fixup_mouse_coords();
                game_states[common_curr_state].tick();
                common_time_for_logic = 0;

                COMMON_to_ortho();
                game_states[common_curr_state].draw();
                COMMON_flip_buffer();
            }

            yield_timeslice();
// #else
/*            if(common_time_for_logic)
            {
                poll_keyboard();
                poll_mouse();
                COMMON_fixup_mouse_coords();
                game_states[common_curr_state].tick();
                common_time_for_logic--;
            }

            COMMON_to_ortho();
            game_states[common_curr_state].draw();
            COMMON_flip_buffer();
#endif */

        }

        game_states[common_curr_state].unload();

        if (common_next_state != STATE_EXIT_APP)
        {
            game_states[common_next_state].load();
            game_states[common_next_state].init();
            common_curr_state_done = FALSE;
        }

        common_curr_state = common_next_state;
    }

    SCRNWIPE_unload();
    PARTICLE_unload();
    MEFX_unload();
    BKGRND_unload();

    COMMON_shutdown();
}
END_OF_MAIN()
