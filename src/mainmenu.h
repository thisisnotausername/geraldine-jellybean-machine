#include    "common.h"
#include    "particles.h"
#include    "mouse_effects.h"
#include    "screenwipe.h"

#ifndef     __MAINMENU_H
    #define __MAINMENU_H
    /******************************************************************************************/
    /*! @defgroup MAINMENU
     * @brief All of the functionality required to paint and interact with the main menu.
     * @{
     */
    void    MAINMENU_init();
    void    MAINMENU_load();
    void    MAINMENU_unload();
    void    MAINMENU_tick();
    void    MAINMENU_draw();
    /*! @} */
#endif
