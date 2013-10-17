/******************************************************************************
*
*  Geraldine's
*     888           888 888         888
*     888  e88~~8e  888 888 Y88b  / 888-~88e   e88~~8e    /~~~8e  888-~88e
*     888 d888  88b 888 888  Y888/  888  888b d888  88b       88b 888  888
*     888 8888__888 888 888   Y8/   888  8888 8888__888  e88~-888 888  888
* |   88P Y888    , 888 888    Y    888  888P Y888    , C888  888 888  888
*  \__8"   "88___/  888 888   /     888-_88"   "88___/   "88_-888 888  888
*                           _/
*      e    e                        888      ,e,
*     d8b  d8b       /~~~8e   e88~~\ 888-~88e  "  888-~88e  e88~~8e
*    d888bdY88b          88b d888    888  888 888 888  888 d888  88b
*   / Y88Y Y888b    e88~-888 8888    888  888 888 888  888 8888__888
*  /   YY   Y888b  C888  888 Y888    888  888 888 888  888 Y888    ,
* /          Y888b  "88_-888  "88__/ 888  888 888 888  888  "88___/
*
*
* Copyright (c) 2013 Clint Rogers & Geraldine Hurley
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*
* * Redistributions of source code must retain the above copyright
*   notice, this list of conditions and the following disclaimer.
* * Redistributions in binary form must reproduce the above
*   copyright notice, this list of conditions and the following disclaimer
*   in the documentation and/or other materials provided with the
*   distribution.
* * Neither the name of the  nor the names of its
*   contributors may be used to endorse or promote products derived from
*   this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* (please see the file COPYING in the top-level directory for more info)
*
* Some things to be aware of:
* - Requires Allegro 4.2.x or 4.4.x (it will not build with 5.x),
*   AllegroGL 0.4.x or higher and DUMB 0.9.x.
*
* - This code grew by accretion, rather than by grand design; a lot of it
*   is bizarrely organised and spectacularly buggy. I am sorry.
*
******************************************************************************/

#include        <stdio.h>
#include        <stdint.h>
#include        <stdlib.h>
#include        <math.h>
#include        <allegro.h>
#include        <aldumb.h>
#include        <alleggl.h>

#ifndef         _WIN32
    #include    <unistd.h>
#endif

#define         glFrustumf      glFrustum
#define         glOrthof        glOrtho

#ifndef         __COMMON
    #define     __COMMON

    #define     DUH_WHERE_AM_I(...) {                                             \
                                        fprintf(stderr,"\n%s, %s(), line %i: ",   \
                                        __FILE__, __FUNCTION__, __LINE__);        \
                                        fprintf(stderr, __VA_ARGS__);             \
                                      }

    #define     OH_SMEG(...)        {                                                          \
                                        fprintf(stderr,"\n\x1b[31m%s, %s(), line %i\x1b[0m: ", \
                                        __FILE__, __FUNCTION__, __LINE__);                     \
                                        fprintf(stderr, __VA_ARGS__);                          \
                                    }

    #define     TO_RADIANS(theta)   (((theta * M_PI) / 180.0))
    #define     TO_DEGREES(theta)   ((theta * (180.0 / M_PI)))

    #ifndef     TRUE
        #define TRUE    1
    #endif

    #ifndef     FALSE
        #define FALSE   0
    #endif

    #define     HOME_PATH_LENGTH        4096

    #define     FONT_PATH     "./gfx/font/gamefont.tga"

    #define     TICKS_PER_SEC   60

    #define     DISPLAY_WIDTH   800
    #define     DISPLAY_HEIGHT  600
    #define     FAR_CLIP_DEPTH  4096.0f

    #define     STATE_EXIT_APP  9999
    #define     STATE_SPLASH    0
    #define     STATE_MENU      1
    #define     STATE_GAMEPLAY  2
    #define     STATE_POEM      3

    #define     DUMB_BUFSIZE    4096
    #define     DUMB_MIXRATE    44100
    #define     DUMB_FADE_MUL   0.96f
    #define     DUMB_FADE_MIN   0.01f
    #define     DUMB_VOL        0.96f       // setting this all the way to 1.0 gives clipping
                                            // with some songs...

    /*! A structure that holds what to do for any of the states
     * the application can be in.  A state is a self-contained area
     * of the application, like the main menu, the high score screen,
     * etc.
     */
    typedef struct
    {
        void    (*init)();
        void    (*load)();
        void    (*unload)();
        void    (*tick)();
        void    (*draw)();
    } GAMESTATE;

    typedef struct
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    }
    VERTEX;

    typedef struct
    {
        GLfloat r;
        GLfloat g;
        GLfloat b;
        GLfloat a;
    } GLCOLOR;

    typedef struct
    {
        GLfloat u;
        GLfloat v;
    }
    TEX_COORD;

    typedef struct
    {
        GLfloat x;
        GLfloat y;
        GLfloat z;
    }
    NORMAL;

    typedef GLuint         GL_FONT;
    typedef GLuint         GLTEXTURE;

    extern float           common_effective_display_width;
    extern float           common_aspect_ratio;
    extern AL_DUH_PLAYER   *common_duhplyr;
    extern int             common_bgm_fade_done;
    extern int             common_curr_state;
    extern int             common_curr_state_done;
    extern int             common_next_state;
    extern int             common_next_state_param;
    extern int             common_have_joysticks;
    extern volatile int    common_time_for_logic;
    extern GL_FONT         common_gamefont;
    extern char            common_user_home_path[];

    void    COMMON_setup(int use_fullscreen);
    GLuint  COMMON_load_texture(const char *filename);
    void    COMMON_glprint(GL_FONT font, GLfloat x, GLfloat y, GLfloat z, GLfloat size, GLfloat spacing, const char *str);
    void    COMMON_look_at(GLfloat cam_x, GLfloat cam_y, GLfloat cam_z, GLfloat dest_x, GLfloat dest_y, GLfloat dest_z);
    void    COMMON_shutdown();
    void    COMMON_fixup_mouse_coords(void);
    void    COMMON_bind_texture(GLuint tex);
    void    COMMON_flip_buffer();
    void    COMMON_show_fps();
    void    COMMON_fade_bgm(void);
    void    COMMON_set_bgm(int track);
    int     COMMON_get_bgm(void);
    void    COMMON_draw_sprite(GLTEXTURE sprite, float x, float  y, float  z, int w, int h);
    void    COMMON_draw_lit_sprite(GLTEXTURE sprite, float x, float  y, float  z, int w, int h,
                GLfloat top_left, GLfloat top_right, GLfloat bot_right, GLfloat bot_left);

#endif
