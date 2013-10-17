#ifndef     __ROUND_START_ANIM_H
   #define  __ROUND_START_ANIM_H

   #include "common.h"
   #include "particles.h"

   void RNDSTRT_load_assets(void);
   void RNDSTRT_unload_assets(void);
   void RNDSTRT_reset(void);
   void RNDSTRT_tick(void);
   void RNDSTRT_paint(void);

   extern uint32_t RNDSTRT_done;

#endif // __ROUND_START_ANIM_H
