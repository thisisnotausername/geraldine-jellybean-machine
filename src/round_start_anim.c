#include "round_start_anim.h"

#define  READY_ANIM_3_END     60
#define  READY_ANIM_2_END     120
#define  READY_ANIM_1_END     180
#define  GO_ANIM_END          240

#define  RNDSTART_PARTICLE_SPAWN 30
#define  RNDSTART_SLIDE_END      36
#define  RNDSTART_ZOOM_START     47
#define  RNDSTART_ZOOM_END       69

#define  RNDSTART_ANNOUNCEMENT_TEX_WIDTH     384
#define  RNDSTART_ANNOUNCEMENT_TEX_HEIGHT    192

#define  READY_GFX_PATH     "./gfx/round/ready.tga"
#define  GO_GFX_PATH        "./gfx/round/go.tga"

#define  READY_CHIME_PATH   "./snd/ready.wav"
#define  GO_CHIME_PATH      "./snd/go.wav"

/// TODO: make this into a VBO
// geometry
VERTEX rndstrt_vert_tmp[4];
TEX_COORD rndstrt_uv_tmp[4];

// images
GLuint rndstrt_ready;
GLuint rndstrt_go;

// sounds
SAMPLE *rndstrt_ready_chime;
SAMPLE *rndstrt_go_chime;

// anim clocks
int rndstrt_tick_count;
int rndstrt_curr_anim_pos;

// world-visible flags
uint32_t RNDSTRT_done;

/**************************************************************************************************/

void RNDSTRT_load_assets(void)
{
    rndstrt_ready  = COMMON_load_texture(READY_GFX_PATH);
    rndstrt_go     = COMMON_load_texture(GO_GFX_PATH);

    if (exists(READY_CHIME_PATH))
        rndstrt_ready_chime = load_sample(READY_CHIME_PATH);
    else
    {
        rndstrt_ready_chime = create_sample(8, 0, 1, 1);
        OH_SMEG("missing sound: %s", READY_CHIME_PATH);
    }

    if (exists(GO_CHIME_PATH))
        rndstrt_go_chime = load_sample(GO_CHIME_PATH);
    else
    {
        rndstrt_go_chime = create_sample(8, 0, 1, 1);
        OH_SMEG("missing sound: %s", GO_CHIME_PATH);
    }

    RNDSTRT_reset();

    rndstrt_uv_tmp[0].u = 0.0f;
    rndstrt_uv_tmp[0].v = 0.0f;

    rndstrt_uv_tmp[1].u = 1.0f;
    rndstrt_uv_tmp[1].v = 0.0f;

    rndstrt_uv_tmp[2].u = 1.0f;
    rndstrt_uv_tmp[2].v = 1.0f;

    rndstrt_uv_tmp[3].u = 0.0f;
    rndstrt_uv_tmp[3].v = 1.0f;
}

/**************************************************************************************************/

void RNDSTRT_unload_assets(void)
{
    glDeleteTextures(1, &rndstrt_ready);
    glDeleteTextures(1, &rndstrt_go);
    destroy_sample(rndstrt_ready_chime);
    destroy_sample(rndstrt_go_chime);
}

/**************************************************************************************************/

void RNDSTRT_reset(void)
{
    rndstrt_tick_count = 0;
    rndstrt_curr_anim_pos = 0;
    RNDSTRT_done = FALSE;
}

/**************************************************************************************************/

void RNDSTRT_tick(void)
{
    if (RNDSTRT_done) return;

    // fire off sounds
    if ((rndstrt_tick_count == 0) |
         (rndstrt_tick_count == READY_ANIM_3_END) |
         (rndstrt_tick_count == READY_ANIM_2_END))
        play_sample(rndstrt_ready_chime, 255, 128, 1000, 0);

    if (rndstrt_tick_count == READY_ANIM_1_END)
        play_sample(rndstrt_go_chime, 255, 128, 1000, 0);

    rndstrt_tick_count++;
    rndstrt_curr_anim_pos++;

    if (rndstrt_tick_count > GO_ANIM_END)
    {
        RNDSTRT_done = TRUE;
        return;
    }

    if ((rndstrt_tick_count > READY_ANIM_1_END) && (rndstrt_curr_anim_pos == RNDSTART_PARTICLE_SPAWN))
    {
        int index;

        for (index = 0; index < 32; index++)
            PARTICLE_spawn((common_effective_display_width / 2.0) - (RNDSTART_ANNOUNCEMENT_TEX_WIDTH / 2.0) + (rand() % RNDSTART_ANNOUNCEMENT_TEX_WIDTH),
                (DISPLAY_HEIGHT / 2.0),
                ((rand() % 27 - 13) / 2.5), ((rand() % 27 - 13) / 2.5), PARTICLE_HEART, (rand() % 64) + 191, rand() % 6);
    }

    if ((rndstrt_tick_count == READY_ANIM_3_END) |
         (rndstrt_tick_count == READY_ANIM_2_END) |
         (rndstrt_tick_count == READY_ANIM_1_END) |
         (rndstrt_tick_count == GO_ANIM_END))
        rndstrt_curr_anim_pos = 0;
}

/**************************************************************************************************/

void RNDSTRT_paint(void)
{
    float y_pos;

    if (RNDSTRT_done) return;

    GLuint curr_anim;

    if (rndstrt_tick_count < GO_ANIM_END)
        curr_anim = rndstrt_go;

    if (rndstrt_tick_count < READY_ANIM_1_END)
        curr_anim = rndstrt_ready;

    if (rndstrt_curr_anim_pos < RNDSTART_SLIDE_END)
        y_pos = (DISPLAY_HEIGHT / 2.0) / rndstrt_curr_anim_pos;
    else
        y_pos = (DISPLAY_HEIGHT / 2.0) / (float)RNDSTART_SLIDE_END;



    rndstrt_vert_tmp[0].x = (common_effective_display_width / 2.0) - (RNDSTART_ANNOUNCEMENT_TEX_WIDTH / 2.0);
    rndstrt_vert_tmp[0].y = (DISPLAY_HEIGHT / 2.0) - (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos;
    rndstrt_vert_tmp[0].z = 0.0;

    rndstrt_vert_tmp[1].x = (common_effective_display_width / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_WIDTH / 2.0);
    rndstrt_vert_tmp[1].y = (DISPLAY_HEIGHT / 2.0) - (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos;
    rndstrt_vert_tmp[1].z = 0.0;

    rndstrt_vert_tmp[2].x = (common_effective_display_width / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_WIDTH / 2.0);
    rndstrt_vert_tmp[2].y = (DISPLAY_HEIGHT / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos;
    rndstrt_vert_tmp[2].z = 0.0;

    rndstrt_vert_tmp[3].x = (common_effective_display_width / 2.0) - (RNDSTART_ANNOUNCEMENT_TEX_WIDTH / 2.0);
    rndstrt_vert_tmp[3].y = (DISPLAY_HEIGHT / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos;
    rndstrt_vert_tmp[3].z = 0.0;

    COMMON_bind_texture(curr_anim);

    if (rndstrt_curr_anim_pos >= RNDSTART_ZOOM_START)
        glColor4f(1.0, 1.0, 1.0, 1.0 - (float)(rndstrt_curr_anim_pos - RNDSTART_ZOOM_START) / (float)(RNDSTART_ZOOM_END - RNDSTART_ZOOM_START));

    else
        glColor4f(1.0, 1.0, 1.0, 1.0);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glVertexPointer(3, GL_FLOAT, 0, rndstrt_vert_tmp);
    glTexCoordPointer(2, GL_FLOAT, 0, rndstrt_uv_tmp);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // draw the countdown text...
    if (rndstrt_tick_count < READY_ANIM_1_END)
    {
        if ((rndstrt_tick_count >= 0) && (rndstrt_tick_count < READY_ANIM_3_END))
            COMMON_glprint(common_gamefont, (common_effective_display_width / 2.0) - 96,
                (DISPLAY_HEIGHT / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos - 32,
                0, 36, -8, "Three...");
        else
        if ((rndstrt_tick_count >= READY_ANIM_3_END) && (rndstrt_tick_count < READY_ANIM_2_END))
            COMMON_glprint(common_gamefont, (common_effective_display_width / 2.0) - 72,
                (DISPLAY_HEIGHT / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos - 32,
                0, 36, -8, "Two...");
        else
        if ((rndstrt_tick_count >= READY_ANIM_2_END) && (rndstrt_tick_count < READY_ANIM_1_END))
            COMMON_glprint(common_gamefont, (common_effective_display_width / 2.0) - 72,
                (DISPLAY_HEIGHT / 2.0) + (RNDSTART_ANNOUNCEMENT_TEX_HEIGHT / 2.0) - y_pos - 32,
                0, 36, -8, "One...");
    }
}
