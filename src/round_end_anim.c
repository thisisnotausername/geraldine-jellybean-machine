#include "round_end_anim.h"

#define  WIN_TEX_WIDTH      512
#define  WIN_TEX_HEIGHT     128

#define  LOSS_TEX_WIDTH     256
#define  LOSS_TEX_HEIGHT    128

#define  WIN_GFX_PATH       "./gfx/round/win.tga"
#define  LOSS_GFX_PATH      "./gfx/round/loss.tga"

#define  WIN_CHIME_PATH       "./snd/win.wav"
#define  LOSS_CHIME_PATH      "./snd/loss.wav"

#define  CLEAR_TIME_FONT_SIZE   28


/// TODO: make this into a VBO
// geometry
VERTEX rndend_vert_tmp[4];
TEX_COORD rndend_uv_tmp[4];

// images
GLuint rndend_win_gfx;
GLuint rndend_loss_gfx;

// sounds
SAMPLE *rndend_win_chime;
SAMPLE *rndend_loss_chime;

// anim clocks
int rndend_tick_count;
int fade_anim_timer;

// internal flags
#define     ROUND_END_IDLE              0
#define     ROUND_END_SLIDING_IN        1
#define     ROUND_END_WAITING_FOR_CLICK 2
#define     ROUND_END_TICK_START        0
#define     ROUND_END_TICK_END          40

int rndend_state;
int rndend_type;

// world-visible flags
uint32_t RNDEND_done;

/**************************************************************************************************/

void RNDEND_start(int type)
{
    rndend_state = ROUND_END_SLIDING_IN;
    rndend_type = type;
}

/**************************************************************************************************/

void RNDEND_load_assets(void)
{
   rndend_win_gfx   = COMMON_load_texture(WIN_GFX_PATH);
   rndend_loss_gfx  = COMMON_load_texture(LOSS_GFX_PATH);

    if (exists(WIN_CHIME_PATH))
        rndend_win_chime = load_sample(WIN_CHIME_PATH);
    else
    {
        rndend_win_chime = create_sample(8, 0, 1, 1);
        OH_SMEG("missing sound: %s", WIN_CHIME_PATH);
    }

    if (exists(LOSS_CHIME_PATH))
        rndend_loss_chime = load_sample(LOSS_CHIME_PATH);
    else
    {
        rndend_loss_chime = create_sample(8, 0, 1, 1);
        OH_SMEG("missing sound: %s", LOSS_CHIME_PATH);
    }

   RNDEND_reset();

   rndend_uv_tmp[0].u = 0.0f;
   rndend_uv_tmp[0].v = 0.0f;

   rndend_uv_tmp[1].u = 1.0f;
   rndend_uv_tmp[1].v = 0.0f;

   rndend_uv_tmp[2].u = 1.0f;
   rndend_uv_tmp[2].v = 1.0f;

   rndend_uv_tmp[3].u = 0.0f;
   rndend_uv_tmp[3].v = 1.0f;
}

/**************************************************************************************************/

void RNDEND_unload_assets(void)
{
    glDeleteTextures(1, &rndend_win_gfx);
    glDeleteTextures(1, &rndend_loss_gfx);
    destroy_sample(rndend_win_chime);
    destroy_sample(rndend_loss_chime);
}

/**************************************************************************************************/

void RNDEND_reset(void)
{
    rndend_tick_count = ROUND_END_TICK_START;
    rndend_state = ROUND_END_IDLE;
    RNDEND_done = FALSE;
    fade_anim_timer = 0;
}

/**************************************************************************************************/

void RNDEND_tick(void)
{
    if (rndend_state == ROUND_END_IDLE) return;

    static int left_clicked = 0;
    static int right_clicked = 0;

    // the mouse keys have to be polled and 'de-bounced' to avoid
    // handling a held button by accident.
    if(mouse_b & 1)
        left_clicked++;
    else
        left_clicked = 0;

    if(mouse_b & 2)
        right_clicked++;
    else
        right_clicked = 0;

    if (rndend_tick_count < ROUND_END_TICK_END)
    {   // still sliding in
        rndend_tick_count++;
    }
    else if (rndend_tick_count == ROUND_END_TICK_END)
    {
        rndend_state = ROUND_END_WAITING_FOR_CLICK;

        if (rndend_type == RNDEND_WIN)
        {
            // won
            int index;

            for (index = 0; index < 200; index++)
            {
                PARTICLE_spawn((common_effective_display_width / 2.0) - (WIN_TEX_WIDTH / 2.0) + (rand() % WIN_TEX_WIDTH),
                    (DISPLAY_HEIGHT / 2.0),
                    ((rand() % 27 - 13) / 2.5), ((rand() % 27 - 13) / 2.5), PARTICLE_HEART, (rand() % 64) + 191, rand() % 6);

                PARTICLE_spawn((common_effective_display_width / 2.0) - (WIN_TEX_WIDTH / 2.0) + (rand() % WIN_TEX_WIDTH),
                    (DISPLAY_HEIGHT / 2.0),
                    ((rand() % 27 - 13) / 1.75), ((rand() % 27 - 13) / 1.75), PARTICLE_SPARK, 223 + (rand() % 32), 5);
            }
            play_sample(rndend_win_chime, 255, 128, 1000, 0);
        }
        else
        {
            // lost
            int index;

            for (index = 0; index < 100; index++)
            {
                PARTICLE_spawn((common_effective_display_width / 2.0) - (LOSS_TEX_WIDTH / 2.0) + (rand() % LOSS_TEX_WIDTH),
                    (DISPLAY_HEIGHT / 2.0),
                    ((rand() % 27 - 13) / 1.95), ((rand() % 27 - 13) / 2.25), PARTICLE_SPARK, 223 + (rand() % 32), 0);
            }
            play_sample(rndend_loss_chime, 255, 128, 1000, 0);
        }

        // advance the clock so this block doesn't execute anymore.
        rndend_tick_count = ROUND_END_TICK_END + 1;
    }
    else
    {
        // awaiting click
        if ((left_clicked == 1) || (right_clicked == 1))
        {
            RNDEND_done = TRUE;
            int index;
            
            if (rndend_type == RNDEND_WIN)
            {

                for (index = 0; index < 80; index++)
                    PARTICLE_spawn((common_effective_display_width / 2.0) - (WIN_TEX_WIDTH / 2.0) + (rand() % WIN_TEX_WIDTH),
                        (DISPLAY_HEIGHT / 2.0),
                        ((rand() % 27 - 13) / 2.5), ((rand() % 27 - 13) / 2.75), PARTICLE_SPARK, 223 + (rand() % 32), 5);
            }
            else
            {

                for (index = 0; index < 80; index++)
                    PARTICLE_spawn((common_effective_display_width / 2.0) - (LOSS_TEX_WIDTH / 2.0) + (rand() % LOSS_TEX_WIDTH),
                        (DISPLAY_HEIGHT / 2.0),
                        ((rand() % 27 - 13) / 2.5), ((rand() % 27 - 13) / 2.75), PARTICLE_SPARK, 223 + (rand() % 32), 5);
            }
        }
    }
}

/**************************************************************************************************/

void RNDEND_paint(void)
{
    char clear_time_buf[HOME_PATH_LENGTH];
    
    if (rndend_state == ROUND_END_IDLE) return;

    GLuint  curr_anim;
    int     curr_anim_width;
    int     curr_anim_height;

    if (rndend_type == RNDEND_WIN)
    {
        curr_anim = rndend_win_gfx;
        curr_anim_width = WIN_TEX_WIDTH;
        curr_anim_height = WIN_TEX_HEIGHT;
    }
    else
    {
        curr_anim = rndend_loss_gfx;
        curr_anim_width = LOSS_TEX_WIDTH;
        curr_anim_height = LOSS_TEX_HEIGHT;
    }

    GLfloat x_pos = (common_effective_display_width / 2.0) -
        (((ROUND_END_TICK_END - rndend_tick_count) / (float)ROUND_END_TICK_END) * ((common_effective_display_width + curr_anim_width) / 2.0));

    if (RNDEND_done)
        return;

    rndend_vert_tmp[0].x = x_pos - (curr_anim_width / 2.0);
    rndend_vert_tmp[0].y = (DISPLAY_HEIGHT / 2.0) - (curr_anim_height / 2.0);
    rndend_vert_tmp[0].z = 0.0;

    rndend_vert_tmp[1].x = x_pos + (curr_anim_width / 2.0);
    rndend_vert_tmp[1].y = (DISPLAY_HEIGHT / 2.0) - (curr_anim_height / 2.0);
    rndend_vert_tmp[1].z = 0.0;

    rndend_vert_tmp[2].x = x_pos + (curr_anim_width / 2.0);
    rndend_vert_tmp[2].y = (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0);
    rndend_vert_tmp[2].z = 0.0;

    rndend_vert_tmp[3].x = x_pos - (curr_anim_width / 2.0);
    rndend_vert_tmp[3].y = (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0);
    rndend_vert_tmp[3].z = 0.0;

    COMMON_bind_texture(curr_anim);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glVertexPointer(3, GL_FLOAT, 0, rndend_vert_tmp);
    glTexCoordPointer(2, GL_FLOAT, 0, rndend_uv_tmp);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    if (rndend_tick_count >= ROUND_END_TICK_END)
    {
        if (rndend_type == RNDEND_WIN)
        {
            snprintf(clear_time_buf, HOME_PATH_LENGTH - 1, "Clear Time: %03.1f sec.", (GMCLK_get_total_time_elapsed() / 10.0f));

            glColor4f(1.0, 1.0, 1.0, 1.0);
            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((CLEAR_TIME_FONT_SIZE - 8) * strlen(clear_time_buf))) / 2.0,
                (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0) + CLEAR_TIME_FONT_SIZE,
                3,
                CLEAR_TIME_FONT_SIZE,
                -8,
                clear_time_buf);
        }

        glColor4f(1, 1, 1, sin(fade_anim_timer / 41.2f) / 2.0f + 0.5);

        if (rndend_type == RNDEND_WIN)
        {
            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((CLEAR_TIME_FONT_SIZE - 8) * 22)) / 2.0,
                (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0) + (CLEAR_TIME_FONT_SIZE * 2.25),
                3,
                CLEAR_TIME_FONT_SIZE,
                -8,
                "Left Click To Continue");
        }
        else
        {
            COMMON_glprint(common_gamefont,
                (common_effective_display_width - ((CLEAR_TIME_FONT_SIZE - 8) * 19)) / 2.0,
                (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0) + (CLEAR_TIME_FONT_SIZE * 2.25),
                3,
                CLEAR_TIME_FONT_SIZE,
                -8,
                "Left Click To Retry");
        }

        COMMON_glprint(common_gamefont,
            (common_effective_display_width - ((CLEAR_TIME_FONT_SIZE - 8) * 33)) / 2.0,
            (DISPLAY_HEIGHT / 2.0) + (curr_anim_height / 2.0) + (CLEAR_TIME_FONT_SIZE * 3.25),
            3,
            CLEAR_TIME_FONT_SIZE,
            -8,
            "Right Click To Return To The Menu");


        fade_anim_timer++;
    }

}

