#include "monster.h"

MONSTER monster_pool[MAX_MONSTERS];
int monster_num_active;

/****************************************************************************************/
/*!
 * @brief Spawns a new monster or does nothing if no more monsters are available.
 */
void MONSTER_spawn(int x, int y)
{
    static int pool_index;
    int num_slots_walked = 0;

    while (num_slots_walked < MAX_MONSTER_SLOTS_TO_WALK)
    {
        // found an unused monster slot?
        if (!monster_pool[pool_index].alive)
        {
            // yes, prefill it with our data.
            monster_pool[pool_index].alive = TRUE;
            monster_pool[pool_index].x = x;
            monster_pool[pool_index].y = y;
            monster_pool[pool_index].frame = 0;
            monster_pool[pool_index].tween = 0;
            
            // move the index to the next, likely-to-be-empty slot
            pool_index++;
            if (pool_index >= MAX_MONSTERS)
                pool_index = 0;
            
            // tell the other parts of the app there's a monster on the loose...
            monster_num_active++;
            
            return;
        }
        
        // not yet, keep walking the pool
        pool_index++;
        if (pool_index >= MAX_MONSTERS)
            pool_index = 0;
            
        // how many slots have we walked? don't get caught in a loop.
        num_slots_walked++;
    }
    
    // didn't find an empty slot in time, give up
    return;
}

/****************************************************************************************/
/*!
 * @brief Runs through all the monsters in the pool and tries to use them to attack 
 * the hero.
 */
void MONSTER_think_all(int player_x, int player_y)
{
    int pool_index;
    
    for (pool_index = 0; pool_index < MAX_MONSTERS; pool_index++)
    {
        if (monster_pool[pool_index].alive)
        {
            // how far away from the goal are we?
            int player_x_dist = monster_pool[pool_index].x - player_x;
            int player_y_dist = monster_pool[pool_index].y - player_y;
            
            // prefer moving along whichever axis has the greatest distance,
            // but if we can't, try the lesser axis...
            if (abs(player_x_dist) > abs(player_y_dist))
            {
                // horizontal first.
                
                // to the left of the player? -------------------
                if (player_x_dist < 0)
                {  
                    if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_LEFT))
                    {
                        // move is valid, record it
                        monster_pool[pool_index].x--;
                    }
                    else // couldn't go left, try up or down if needed
                    {
                        // up
                        if (player_y_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_UP))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].y--;
                            }
                        }
                        
                        // down
                        if (player_y_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_DOWN))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].y++;
                            }
                        }
                    }
                }
                
                // to the right of the player? -------------------
                if (player_x_dist > 0)
                {  
                    if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_RIGHT))
                    {
                        // move is valid, record it
                        monster_pool[pool_index].x++;
                    }
                    else // couldn't go left, try up or down if needed
                    {
                        // up
                        if (player_y_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_UP))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].y--;
                            }
                        }
                        
                        // down
                        if (player_y_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_DOWN))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].y++;
                            }
                        }
                    }
                }
            }
            else
            {
                // vertical
                
                // above the player? -------------------
                if (player_y_dist < 0)
                {  
                    if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_UP))
                    {
                        // move is valid, record it
                        monster_pool[pool_index].y--;
                    }
                    else // couldn't go up, try left or right if needed
                    {
                        // left
                        if (player_x_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_LEFT))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].x--;
                            }
                        }
                        
                        // right
                        if (player_x_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_RIGHT))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].x++;
                            }
                        }
                    }
                }
                
                // below the player? -------------------
                if (player_y_dist > 0)
                {  
                    if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_DOWN))
                    {
                        // move is valid, record it
                        monster_pool[pool_index].y++;
                    }
                    else // couldn't go up, try left or right if needed
                    {
                        // left
                        if (player_x_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_LEFT))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].x--;
                            }
                        }
                        
                        // right
                        if (player_x_dist < 0)
                        {
                            if (puzgrid_swap_gems(monster_pool[pool_index].x, monster_pool[pool_index].y, PUZGRID_ANIM_RIGHT))
                            {
                                // move is valid, record it
                                monster_pool[pool_index].x++;
                            }
                        }
                    }
                }
            }
        }
        
        // still to come - check whether we're in an adjacent square to the player,
        // in which case, we should die and subtract time from them.
    }
}
