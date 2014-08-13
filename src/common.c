/*******************************************************************************
*
* Part of GAME NAME - see common.h for copyright info.
*
*******************************************************************************/

#include "common.h"

#define GLYPHS_PER_ROW          16
#define GLYPH_WIDTH             0.0625f
#define GLYPH_HEIGHT            0.125f
#define GLYPH_SCREEN_WIDTH      16.0f
#define FIRST_GLYPH             32

/******************************************************************************/

DUH             *common_duh             = NULL;
AL_DUH_PLAYER   *common_duhplyr         = NULL;
float           common_duhvol           = DUMB_VOL;
int             common_curr_bgm         = 0;
int             common_should_fade_bgm;

int             common_bgm_fade_done    = FALSE;
int             common_curr_state;
int             common_next_state;
int             common_next_state_param;
volatile int    common_frames_this_sec;
volatile int    common_frames_per_sec;
volatile int    common_time_for_logic;
int             common_curr_state_done  = FALSE;
int             common_have_joysticks   = FALSE;

float           common_aspect_ratio;
float           common_effective_display_width;

char            common_user_home_path[HOME_PATH_LENGTH];

GLuint          texture_name = 1;
GLuint          common_gamefont;

/******************************************************************************/
/*!
 * @brief Reports how many frames we were able to draw in the last second.
 * It is bound to a timer and should never be called manually.
 */
void sample_framerate(void)
{
    common_frames_per_sec  = common_frames_this_sec;
    common_frames_this_sec = 0;
}
END_OF_FUNCTION(sample_framerate);

/******************************************************************************/
/*!
 * @brief Advances the game logic at least one tick, or more if a repaint takes too
 * long. Called automatically as a timer task, should never be called manually.
 *
 * To use this:
 * In the game loop, we have something like
 * while(1)
 * {
 *      if(time_for_logic)
 *      {
 *          do_something_awesome();
 *          time_for_logic--;
 *          if(time_for_logic == 0)
 *          {
 *              draw_something_awesome();
 *          }
 *      }
 *      else
 *      {
 *          rest(1); // don't hog the cpu when not needed
 *      }
 * }
 */
void game_heartbeat()
{
//#ifdef _WIN32
    common_time_for_logic = 1;
//#else
//    common_time_for_logic++;
//#endif
}
END_OF_FUNCTION(game_heartbeat);

/******************************************************************************/
/*! @brief Display a string at the specified location, in the specified font.
 *
 * @note This function doesn't accept formatting commands; to use them, you'll
 * have to snprintf a string together first, then pass that.
 *
 * @note There isn't any support for characters outside of ASCII currently;
 * if someone reading this has an idea of how to extend it for CKJV and
 * accent support, let me know!
 */

void COMMON_glprint(GL_FONT font, GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat spacing, const char *str)
{
    int         row;
    int         col;
    int         index;
    VERTEX      verts[4];
    TEX_COORD   uvs[4];

    COMMON_bind_texture(font);

    glDisableClientState(GL_COLOR_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2,GL_FLOAT,0,uvs);
    glVertexPointer(3,GL_FLOAT,0,verts);

    for(index = 0; index < strlen(str); index++)
    {
        row = (str[index] - FIRST_GLYPH) / GLYPHS_PER_ROW;
        col = (str[index] - FIRST_GLYPH) % GLYPHS_PER_ROW;

        verts[0].x = x  + (index * (size + spacing));
        verts[0].y = y;
        verts[0].z = z;

        uvs[0].u = col * GLYPH_WIDTH;
        uvs[0].v = row * GLYPH_HEIGHT;

        verts[1].x = verts[0].x + size;
        verts[1].y = y;
        verts[1].z = z;

        uvs[1].u = (col + 1) * GLYPH_WIDTH;
        uvs[1].v = row * GLYPH_HEIGHT;

        verts[2].x = verts[0].x + size;
        verts[2].y = y + size;
        verts[2].z = z;

        uvs[2].u = (col + 1) * GLYPH_WIDTH;
        uvs[2].v = (row + 1) * GLYPH_HEIGHT;

        verts[3].x = verts[0].x;
        verts[3].y = y + size;
        verts[3].z = z;

        uvs[3].u = col * GLYPH_WIDTH;
        uvs[3].v = (row + 1) * GLYPH_HEIGHT;

        glDrawArrays(GL_TRIANGLE_FAN,0,4);
    }
}

/*****************************************************************************/
/*!
 * @brief OpenGL-friendly replacement for draw_sprite().
 *
 * @param sprite The texture this sprite should have.
 * @param x The horizontal component of this sprite's location in space.
 * @param y The vertical component of this sprite's location in space.
 * @param z The depth component of this sprite's location in space.
 * @param w The width (in GL units) this sprite should be drawn with.
 * @param h The height (in GL units) this sprite should be drawn with.
 */
void COMMON_draw_sprite(GLTEXTURE sprite, float  x, float  y, float  z, int w, int h)
{
    VERTEX verts[4];
    TEX_COORD uvs[4];

    COMMON_bind_texture(sprite);
    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2,GL_FLOAT,0,uvs);
    glVertexPointer(3,GL_FLOAT,0,verts);

    verts[0].x = x;
    verts[0].y = y;
    verts[0].z = z;

    uvs[0].u = 0;
    uvs[0].v = 0;

    verts[1].x = x + w;
    verts[1].y = y;
    verts[1].z = z;

    uvs[1].u = 1;
    uvs[1].v = 0;

    verts[2].x = x + w;
    verts[2].y = y + h;
    verts[2].z = z;

    uvs[2].u = 1;
    uvs[2].v = 1;

    verts[3].x = x;
    verts[3].y = y + h;
    verts[3].z = z;

    uvs[3].u = 0;
    uvs[3].v = 1;

    glDrawArrays(GL_TRIANGLE_FAN,0,4);
}

/*****************************************************************************/
/*!
 * @brief OpenGL-friendly replacement for draw_lit_sprite().
 *
 * @param sprite The texture this sprite should have.
 * @param x The horizontal component of this sprite's location in space.
 * @param y The vertical component of this sprite's location in space.
 * @param z The depth component of this sprite's location in space.
 * @param top_left How intensely the top left corner is to be lit.
 * @param top_right How intensely the top right corner is to be lit.
 * @param bot_left How intensely the bottom left corner is to be lit.
 * @param bot_right How intensely the bottom left corner is to be lit.
 * @bug Only white lighting is supported currently.
 */
void COMMON_draw_lit_sprite(GLTEXTURE sprite, float  x, float  y, float  z, int w, int h,
    GLfloat top_left, GLfloat top_right, GLfloat bot_right, GLfloat bot_left)
{
    VERTEX verts[6];
    TEX_COORD uvs[6];
    GLCOLOR cols[6];

    COMMON_bind_texture(sprite);
    glEnableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glEnable(GL_TEXTURE_2D);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);
    glVertexPointer(3, GL_FLOAT, 0, verts);
    glColorPointer(4, GL_FLOAT, 0, cols);

    verts[0].x = x + (w / 2.0);
    verts[0].y = y + (h / 2.0);
    verts[0].z = z;

    uvs[0].u = 0.5;
    uvs[0].v = 0.5;

    cols[0].r = (top_left + top_right + bot_right + bot_left) / 4.0;
    cols[0].g = cols[0].r;
    cols[0].b = cols[0].r;
    cols[0].a = 1;

    verts[1].x = x;
    verts[1].y = y;
    verts[1].z = z;

    uvs[1].u = 0;
    uvs[1].v = 0;

    cols[1].r = top_left;
    cols[1].g = top_left;
    cols[1].b = top_left;
    cols[1].a = 1;

    verts[2].x = x + w;
    verts[2].y = y;
    verts[2].z = z;

    uvs[2].u = 1;
    uvs[2].v = 0;

    cols[2].r = top_right;
    cols[2].g = top_right;
    cols[2].b = top_right;
    cols[2].a = 1;

    verts[3].x = x + w;
    verts[3].y = y + h;
    verts[3].z = z;

    uvs[3].u = 1;
    uvs[3].v = 1;

    cols[3].r = bot_right;
    cols[3].g = bot_right;
    cols[3].b = bot_right;
    cols[3].a = 1;

    verts[4].x = x;
    verts[4].y = y + h;
    verts[4].z = z;

    uvs[4].u = 0;
    uvs[4].v = 1;

    cols[4].r = bot_left;
    cols[4].g = bot_left;
    cols[4].b = bot_left;
    cols[4].a = 1;

    verts[5].x = verts[1].x ;
    verts[5].y = verts[1].y ;
    verts[5].z = verts[1].z ;

    uvs[5].u   = uvs[1].u   ;
    uvs[5].v   = uvs[1].v   ;

    cols[5].r  = cols[1].r  ;
    cols[5].g  = cols[1].g  ;
    cols[5].b  = cols[1].b  ;
    cols[5].a  = cols[1].a  ;

    glDrawArrays(GL_TRIANGLE_FAN,0,6);
}

/*****************************************************************************/
/*!
 * @brief Prepare the sound, music, input, timer and graphics subsystems for use.
 * @param use_fullscreen Whether the game should attempt to go fullscreen, or
 *      display itself in a window.
 */
void COMMON_setup(int use_fullscreen)
{
    srand((unsigned) time(NULL));

    allegro_init();
    install_allegro_gl();
    install_sound(DIGI_AUTODETECT, MIDI_NONE, NULL);
    install_keyboard();
    install_mouse();

    if(install_joystick(JOY_TYPE_AUTODETECT) == 0)
        common_have_joysticks = TRUE;

    atexit(&dumb_exit);
    dumb_register_stdfiles();

    // Get user's home directory and where we're being run from
    // I hope this doesn't end up on tdwtf, but I have no idea what else to do...
#ifdef  __linux__
    strncpy(common_user_home_path, getenv("HOME"), HOME_PATH_LENGTH - 1);

    // Workaround defective WMs/DEs that don't automatically change to the right
    // directory if we're double-clicked (yes, you, LXDE, you piece of sh...)
    char EXE_PATH_TMP[HOME_PATH_LENGTH];
    get_executable_name(EXE_PATH_TMP, HOME_PATH_LENGTH - 1);
    chdir((char *) dirname(EXE_PATH_TMP));
#else
#ifdef _WIN32
    strncpy(common_user_home_path, getenv("USERPROFILE"), HOME_PATH_LENGTH - 1);
#endif
#endif

    // Set up the display.
    // Compute the correct aspect ratio for the user's display.
    int width_tmp;
    int height_tmp;

    if((!use_fullscreen) || (get_desktop_resolution(&width_tmp, &height_tmp) != 0))
    {
        width_tmp  = DISPLAY_WIDTH;
        height_tmp = DISPLAY_HEIGHT;
    }

    common_aspect_ratio = (float)width_tmp / (float)height_tmp;
    common_effective_display_width =
        DISPLAY_WIDTH * (common_aspect_ratio / ((float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT));

    // Choose a colour depth.
    if(desktop_color_depth() != 0)
        set_color_depth(desktop_color_depth());
    else
        set_color_depth(16);

    // Suggest a good screen mode for OpenGL.
    allegro_gl_set(AGL_Z_DEPTH, get_color_depth());
    allegro_gl_set(AGL_COLOR_DEPTH, get_color_depth());
    allegro_gl_set(AGL_FULLSCREEN, use_fullscreen);
    allegro_gl_set(AGL_DOUBLEBUFFER, TRUE);
    allegro_gl_set(AGL_SUGGEST,
        AGL_FULLSCREEN | AGL_DOUBLEBUFFER | AGL_Z_DEPTH | AGL_COLOR_DEPTH);

    // Set the graphics mode.
    set_gfx_mode(GFX_OPENGL, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0, 0);

    // Set the font
    if(exists(FONT_PATH))
    {
        common_gamefont = COMMON_load_texture(FONT_PATH);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    }

    // Set some basic GL settings for speed
    glFogf(GL_FOG_MODE, GL_LINEAR);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_FASTEST);
    glHint(GL_FOG_HINT, GL_FASTEST);

    // Set up the heartbeat
    common_time_for_logic = 0;
    LOCK_VARIABLE(common_time_for_logic);
    LOCK_FUNCTION(game_heartbeat);

    install_int_ex(game_heartbeat, BPS_TO_TIMER(TICKS_PER_SEC));

    // Set up the performance timer
    LOCK_VARIABLE(common_frames_per_sec);
    LOCK_VARIABLE(common_frames_this_sec);
    LOCK_FUNCTION(sample_framerate);

    install_int_ex(sample_framerate,BPS_TO_TIMER(1));

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glAlphaFunc(GL_GREATER, 0.5);
}

/*****************************************************************************/
/*!
 * @brief glflush() everything and copy the current contents of the double
 * buffer to the display.
 *
 * We also poll the music player and handle volume fades here. This is because
 * both have to be called regularly many times a second, and redrawing has
 * to happen many times a second, so this seemed like the least worst place
 * to put them.
 */
void COMMON_flip_buffer()
{
    if(common_duhplyr != NULL)
    {
        // check if a music change is pending
        if(common_should_fade_bgm)
        {
            // yes, fade out
            common_duhvol = common_duhvol * DUMB_FADE_MUL;
            al_duh_set_volume(common_duhplyr, common_duhvol);

            if(common_duhvol < DUMB_FADE_MIN)
                common_bgm_fade_done = TRUE;
        }
        al_poll_duh(common_duhplyr);
    }

    allegro_gl_flip();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    common_frames_this_sec++;
}

/*****************************************************************************/
/*! @brief Show the number of frames drawn in the last second in the upper
 * left corner. For debugging, shouldn't be called in enduser-facing releases.
 */
void COMMON_show_fps()
{
    char buffer[16];

    if(common_frames_per_sec < (TICKS_PER_SEC / 2))
        glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
    else if(common_frames_per_sec < (TICKS_PER_SEC / 1.5))
        glColor4f(1.0f, 1.0f, 0.0f, 1.0f);
    else
        glColor4f(0.0f, 1.0f, 0.0f, 1.0f);

    snprintf(buffer, 16, "FPS: %03d", common_frames_per_sec);
    COMMON_glprint(common_gamefont, 8, 8, 0, 16, 0, buffer);
}

/******************************************************************************/
/*! @brief Load an image via Allegro and convert it into a ready-for-use texture.
 * @note Instead of crashing or asserting if it can't load an image, it'll instead
 * create a really-ugly bright green texture and print an error to the console.
 */
GLuint COMMON_load_texture(const char *filename)
{
    int format;
    GLuint ret_val;

    BITMAP *b;
    set_color_depth(24);
    set_color_conversion(COLORCONV_KEEP_ALPHA);

    if (exists(filename))
        b = load_bitmap(filename,NULL);
    else
    {
        OH_SMEG("missing image: %s", filename);
        b = create_bitmap_ex(24,1,1);
        putpixel(b,0,0,makecol24(0,255,0));
    }

    switch(bitmap_color_depth(b))
    {
        case 24:    format = GL_RGB;
                    break;

        case 32:    format = GL_RGBA;
                    break;

        case 16:    format = GL_RGB;
                    break;

        case 8:     format = GL_LUMINANCE;
                    break;

        default:   format = GL_RGB;
    }

    glBindTexture(GL_TEXTURE_2D, texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, format, b->w, b->h, 0, format, GL_UNSIGNED_BYTE, b->line[0]);

    destroy_bitmap(b);

    /// TODO: how come this doesn't use glGenTextures()?
    ret_val = texture_name;
    texture_name++;

    return ret_val;
}

/******************************************************************************/
/*! @brief Switch to a perspective projection. Most 3D drawing should be done
 * in this mode.
 */
void COMMON_to_perspective(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glFrustumf(-1.0f * common_aspect_ratio, 1.0f * common_aspect_ratio,
                -1.0f, 1.0f,
                2.0f, FAR_CLIP_DEPTH);

    glEnable(GL_DEPTH_TEST);
    glMatrixMode(GL_MODELVIEW);
}

/******************************************************************************/
/*! @brief Switch to an orthographic projection and settings useful for drawing
 * dialog boxes, a HUD, a 2D-game, etc.
 *
 * @note the top left is the origin.
 */
void COMMON_to_ortho(void)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrthof(0.0f, (float)DISPLAY_HEIGHT * common_aspect_ratio,
            (float)DISPLAY_HEIGHT, 0.0f,
            -10.0f, 100.0f);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

/******************************************************************************/
/*! @brief Stupid but effective replacement for gluLookAt() to ease porting to
 * OpenGL ES.
 */
void COMMON_look_at(GLfloat cam_x, GLfloat cam_y, GLfloat cam_z,
                GLfloat dest_x, GLfloat dest_y, GLfloat dest_z)
{
    float x_dist = dest_x - cam_x;
    float y_dist = dest_y - cam_y;
    float z_dist = dest_z - cam_z;

    float y_rot_angle = atan2(-x_dist, -z_dist);
    float x_rot_angle = atan2( y_dist,hypotf(x_dist, z_dist));

    glRotatef(TO_DEGREES(-x_rot_angle), 1, 0, 0);
    glRotatef(TO_DEGREES(-y_rot_angle), 0, 1, 0);

    glTranslatef(-cam_x,-cam_y,-cam_z);
}

/******************************************************************************/
/*! @brief Turn off sound, music, timer, input and graphics subsystems and
 * cleans up. Should be called right before the end of main().
 */
void COMMON_shutdown(void)
{
    fprintf(stderr,"\n");
    clear_keybuf();
    remove_int(game_heartbeat);
    remove_int(sample_framerate);
    glDeleteTextures(1, &common_gamefont);
    set_gfx_mode(GFX_TEXT,0,0,0,0);
}

/******************************************************************************/
/*! @brief Wrapper for glBindTexture(). Internally, it tracks the currently-
 * bound texture and tries to skip out on binding the same texture twice to
 * save on state changes.
 */
void COMMON_bind_texture(GLuint tex)
{
    static GLuint prev_tex = 0;

    if(tex != prev_tex)
    {
        glBindTexture(GL_TEXTURE_2D, tex);
        prev_tex = tex;
    }
}

/******************************************************************************************/
/*! @brief Change what music is playing. To stop BGM entirely, pass a non-existant
 * track ID (usually 0 if you've set up your bgm/ directory right).
 */
void COMMON_set_bgm(int track)
{
    char buffer[2048];

    // already playing this, don't need to do anything
    if(track == common_curr_bgm) return;

    common_curr_bgm = track;

    // check whether requested music exists
    sprintf(buffer,"./bgm/%02d.xm",track);
    if(exists(buffer))
    {
        al_stop_duh(common_duhplyr);

        // yes, load it and start playing
        common_duh             = dumb_load_xm_quick(buffer);
        common_duhplyr         = al_start_duh(common_duh, 2, 0, DUMB_VOL, DUMB_BUFSIZE, DUMB_MIXRATE);
        common_bgm_fade_done   = FALSE;
        common_duhvol          = DUMB_VOL;
        common_should_fade_bgm = FALSE;
    }
    else
    {
        // no - null out duh and player
        al_stop_duh(common_duhplyr);
        common_duhplyr = NULL;
    }
}

/******************************************************************************************/
/*! @brief Cause the currently-playing track to fade out.
 *  Returns immediately and does not block; the actual act of fading happens
 *  in COMMON_flip_buffer.
 */
void COMMON_fade_bgm(void)
{
    common_should_fade_bgm = TRUE;
}

/******************************************************************************************/
/*! @brief Get the track ID of the currently-playing BGM.
 */
int COMMON_get_bgm(void)
{
    return common_curr_bgm;
}

/******************************************************************************************/
/*! @brief Because we could be running in 'fake' widescreen mode, 'mouse_x' might need to be
 *  adjusted to match the aspect ratio of the projection...
 */
void COMMON_fixup_mouse_coords(void)
{
    mouse_x *= common_aspect_ratio / ((float)DISPLAY_WIDTH / (float)DISPLAY_HEIGHT);
}
