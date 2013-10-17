#include "gameclock.h"

#define HOURGLASS_GFX_PATH      "./gfx/timer/hourglass.tga"
#define SAND_TOP_GFX_PATH       "./gfx/timer/sand-top.tga"
#define SAND_BOTTOM_GFX_PATH    "./gfx/timer/sand-bottom.tga"

#define TICKS_TO_COUNT          6 //!< number of ticks in 1/10th sec

#define STRING_POOL_SIZE        4
#define STRING_LENGTH           10

#define TIME_TO_LIVE            120
#define GLYPH_SIZE              28
#define GLYPH_SPACING           -8

#define NUMERIC_TIME_SIZE       24
#define NUMERIC_TIME_SPACING    -6

#define HOURGLASS_X             (common_effective_display_width - 160)
#define HOURGLASS_Y             32
#define HOURGLASS_W             128
#define HOURGLASS_H             256

#define SAND_TOP_START          (29.0 / 256.0)  //!< this is based on the hourglass sprite and mustn't be changed
#define SAND_BOT_END            (226.0 / 256.0) //!< this is based on the hourglass sprite and mustn't be changed
#define SAND_ACTUAL_LENGTH      (1.0 - SAND_TOP_START)

#define MAX_TIME                480

/*******************************************************************************************/

typedef struct
{
    int     time_to_live;
    char    text[STRING_LENGTH];
} GMCLK_ANNOUNCEMENT;

/*******************************************************************************************/

GLuint gmclk_hourglass_gfx;
GLuint gmclk_sandtop_gfx;
GLuint gmclk_sandbot_gfx;

int gmclk_ticks;    //!< used to count off the number of logic updates before advancing the clock
int gmclk_time_elapsed;
int gmclk_time_remaining;

int GMCLK_should_count_down = 0;

GMCLK_ANNOUNCEMENT gmclk_announcement_pool[STRING_POOL_SIZE];

/*******************************************************************************************/
/*! @brief Doxygen me, please.
 */
GMCLK_spawn_announcement(const char *text)
{
    static int last_used = 0;

    strncpy(gmclk_announcement_pool[last_used].text, text, STRING_LENGTH - 1);
    gmclk_announcement_pool[last_used].time_to_live = TIME_TO_LIVE;

    last_used++;
    if (last_used >= STRING_POOL_SIZE) last_used = 0;
}
/*******************************************************************************************/
/*! @brief Load all the assets needed to draw the timer.
 */
void GMCLK_load(void)
{
    gmclk_hourglass_gfx = COMMON_load_texture(HOURGLASS_GFX_PATH);
    gmclk_sandtop_gfx   = COMMON_load_texture(SAND_TOP_GFX_PATH);
    gmclk_sandbot_gfx   = COMMON_load_texture(SAND_BOTTOM_GFX_PATH);
}

/*******************************************************************************************/
/*! @brief Unload all assets and free up texture memory.
 */
void GMCLK_unload(void)
{
    glDeleteTextures(1, &gmclk_hourglass_gfx);
    glDeleteTextures(1, &gmclk_sandtop_gfx  );
    glDeleteTextures(1, &gmclk_sandbot_gfx  );
}

/******************************************************************************************/
/*! @brief Advances the time remaining and time elapsed counters
 * by one logic tick.
 */
void GMCLK_tick(void)
{
    gmclk_ticks++;

    if((gmclk_ticks >= TICKS_TO_COUNT) && (GMCLK_should_count_down))
    {
        gmclk_ticks = 0;
        gmclk_time_elapsed++;
        gmclk_time_remaining--;
    }

    if (gmclk_time_remaining < 0)
        gmclk_time_remaining = 0;

    int index;
    for (index = 0; index < STRING_POOL_SIZE; index++)
    {
        if (gmclk_announcement_pool[index].time_to_live > 0)
            gmclk_announcement_pool[index].time_to_live--;
    }
}

/******************************************************************************************/
/*! @brief Adds the specified amount of time to the time-remaining counter;
 * pass a negative value to subtract time.
 *
 * @param amount The number of tenths of a second to add.
 */
void GMCLK_add(int amount)
{
    char buf[STRING_LENGTH];
    gmclk_time_remaining += amount;

    if (gmclk_time_remaining > MAX_TIME) gmclk_time_remaining = MAX_TIME;

    if (amount > 0)
    {
        snprintf(buf, STRING_LENGTH-1, "+%1.1f sec", amount / 10.0f);
        GMCLK_spawn_announcement(buf);
    }
}

/******************************************************************************************/
/*! @brief Returns the amount of time remaining in the round.
 */
int GMCLK_get_time_remaining(void)
{
    return gmclk_time_remaining;
}

/******************************************************************************************/
/*! @brief Returns the total amount of wall time that has elapsed during play this round.
 */
int GMCLK_get_total_time_elapsed(void)
{
    return gmclk_time_elapsed;
}

/******************************************************************************************/
/*! @brief Reset the time-elapsed counter to 0 and the time-remaining counter
 * to the specified value. Usually should only be called at the start of a round.
 *
 * @param amount The number of tenths of a second to start the round with.
 */
void GMCLK_reset(int amount)
{
    gmclk_time_remaining = amount;
    gmclk_time_elapsed = 0;
    gmclk_ticks = 0;
    GMCLK_should_count_down = TRUE;

    int index;
    for (index = 0; index < STRING_POOL_SIZE; index++)
        gmclk_announcement_pool[index].time_to_live = 0;
}

/******************************************************************************************/
/*! @brief Displays the round timer. (currently placeholder, needs art)
 */
void GMCLK_draw(void)
{
    VERTEX      verts[4];   /// TODO: make these into a VBO
    TEX_COORD   uvs[4];
    char        buf[16];
    float       amt_to_skip;

    glColor3ub(255, 255, 255);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexCoordPointer(2,GL_FLOAT,0,uvs);
    glVertexPointer(3,GL_FLOAT,0,verts);

    // sand top
    COMMON_bind_texture(gmclk_sandtop_gfx);
    amt_to_skip = (float) gmclk_time_remaining / MAX_TIME;
    if (amt_to_skip > 1.0f) amt_to_skip = 1.0f;
    if (amt_to_skip < 0.0f) amt_to_skip = 0.0f;

    // compensate for the offset into the hourglass sprite
    amt_to_skip = (amt_to_skip * SAND_ACTUAL_LENGTH) + SAND_TOP_START;

    verts[0].x = HOURGLASS_X;
    verts[0].y = HOURGLASS_Y + (HOURGLASS_H * 0.5) - (amt_to_skip * HOURGLASS_H * 0.5);
    verts[0].z = 0;

    uvs[0].u = 0;
    uvs[0].v = 0.5 - (amt_to_skip * 0.5);

    verts[1].x = HOURGLASS_X + HOURGLASS_W;
    verts[1].y = HOURGLASS_Y + (HOURGLASS_H * 0.5) - (amt_to_skip * HOURGLASS_H * 0.5);
    verts[1].z = 0;

    uvs[1].u = 1;
    uvs[1].v = 0.5 - (amt_to_skip * 0.5);

    verts[2].x = HOURGLASS_X;
    verts[2].y = HOURGLASS_Y + HOURGLASS_H;
    verts[2].z = 0;

    uvs[2].u = 0;
    uvs[2].v = 1;

    verts[3].x = HOURGLASS_X + HOURGLASS_W;
    verts[3].y = HOURGLASS_Y + HOURGLASS_H;
    verts[3].z = 0;

    uvs[3].u = 1;
    uvs[3].v = 1;

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    // sand bottom
    COMMON_bind_texture(gmclk_sandbot_gfx);

    verts[0].x = HOURGLASS_X;
    verts[0].y = HOURGLASS_Y + (HOURGLASS_H * 0.5) + (amt_to_skip * HOURGLASS_H * 0.5);
    verts[0].z = 0;

    uvs[0].u = 0;
    uvs[0].v = 0.5 + (amt_to_skip * 0.5);

    verts[1].x = HOURGLASS_X + HOURGLASS_W;
    verts[1].y = HOURGLASS_Y + (HOURGLASS_H * 0.5) + (amt_to_skip * HOURGLASS_H * 0.5);
    verts[1].z = 0;

    uvs[1].u = 1;
    uvs[1].v = 0.5 + (amt_to_skip * 0.5);

    verts[2].x = HOURGLASS_X;
    verts[2].y = HOURGLASS_Y + HOURGLASS_H;
    verts[2].z = 0;

    uvs[2].u = 0;
    uvs[2].v = 1;

    verts[3].x = HOURGLASS_X + HOURGLASS_W;
    verts[3].y = HOURGLASS_Y + HOURGLASS_H;
    verts[3].z = 0;

    uvs[3].u = 1;
    uvs[3].v = 1;

    glDrawArrays(GL_TRIANGLE_STRIP,0,4);

    // hourglass
    COMMON_draw_sprite(gmclk_hourglass_gfx, HOURGLASS_X, HOURGLASS_Y, 0, HOURGLASS_W, HOURGLASS_H);

    // number of seconds left numerically
    snprintf(buf, 15, "%02.1f sec.", gmclk_time_remaining / 10.0f);

    // colour goes reddish at 10 sec remaining, full red at 5 sec
    if (gmclk_time_remaining < 51)
        glColor4ub(255,64,64,192);
    else if (gmclk_time_remaining < 101)
        glColor4ub(255,160,160,192);
    else
        glColor4ub(255,255,255,192);

    COMMON_glprint(common_gamefont,
        HOURGLASS_X - NUMERIC_TIME_SIZE,
        HOURGLASS_H + HOURGLASS_Y + 2, -1, NUMERIC_TIME_SIZE, NUMERIC_TIME_SPACING,
        buf);

    // announcement text
    int index;
    int pos_tmp;
    for (index = 0; index < STRING_POOL_SIZE; index++)
    {
        if (gmclk_announcement_pool[index].time_to_live > 0)
        {
            pos_tmp = ((TIME_TO_LIVE - gmclk_announcement_pool[index].time_to_live) * 7);
            if (pos_tmp > ((GLYPH_SIZE + GLYPH_SPACING) * STRING_LENGTH))
                pos_tmp = ((GLYPH_SIZE + GLYPH_SPACING) * STRING_LENGTH);

            // fade out older ones
            if (gmclk_announcement_pool[index].time_to_live < 64)
                glColor4ub(255,255,255, gmclk_announcement_pool[index].time_to_live * 4);
            else
                glColor4ub(255,255,255,255);

            COMMON_glprint(common_gamefont,
                common_effective_display_width - pos_tmp,
                HOURGLASS_H + HOURGLASS_Y + ((index + 2) * GLYPH_SIZE), -1, GLYPH_SIZE, GLYPH_SPACING,
                gmclk_announcement_pool[index].text);
        }
    }
}
