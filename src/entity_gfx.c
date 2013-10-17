/*******************************************************************************
*
* Part of GAME NAME - see common.h for copyright info.
*
*******************************************************************************/

#include "entity_gfx.h"

char *entgfx_pathnames[] = 
{
    "idle", "left", "right", "up", "down", "win", "lose"
};

ENTITY_GFX   entgfx_princess;
ENTITY_GFX   entgfx_monster;

void ENTGFX_load(const char *foldername, ENTITY_GFX *dest) 
{
    char buf[4096];
    int path_index;
    int frame_index;
    
    for (path_index = 0; path_index < MAX_ACTIONS; path_index++)
    {
        for (frame_index = 0; frame_index < MAX_FRAMES; frame_index++)
        {
            snprintf(buf, 4095, "%s/%s/%02d.tga", foldername, entgfx_pathnames[path_index],
                frame_index);
            if (exists(buf)) {
                dest->frame_data[path_index][frame_index] = COMMON_load_texture(buf);
                dest->frame_counts[path_index]++;
            }
        }
    }
}

/******************************************************************************/
/*!
 * @brief need docs, plox
 */
void ENTGFX_unload(ENTITY_GFX *dest)
{
    int path_index;
    int frame_index;
    
    for (path_index = 0; path_index < MAX_ACTIONS; path_index++)
    {
        for (frame_index = 0; frame_index < MAX_FRAMES; frame_index++)
            glDeleteTextures(1, &dest->frame_data[path_index][frame_index]);

        dest->frame_counts[path_index] = 0;
    }
}
