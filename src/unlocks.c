#include    "unlocks.h"

#define     SAVEGAME_FILENAME   ".jellybean_unlocked"

char unlockables[NUM_UNLOCK_DIFFICULTIES];

/**************************************************************************************************/
/*!
 * @brief Load a file telling us what's unlocked already.
 */
void UNLOCKED_load(void)
{
    char buf[HOME_PATH_LENGTH + 19]; // ".jellybean_unlocked" is 19 characters long
    FILE *fin;

    // make sure the first level defaults to unlocked, no matter what.
    if (unlockables[UNLOCKS_EASY]   < 1) unlockables[UNLOCKS_EASY]   = 1;
    if (unlockables[UNLOCKS_MEDIUM] < 1) unlockables[UNLOCKS_MEDIUM] = 1;
    if (unlockables[UNLOCKS_HARD]   < 1) unlockables[UNLOCKS_HARD]   = 1;

    // actually try to load the file.
    snprintf(buf, sizeof(buf) - 1, "%s/%s", common_user_home_path, SAVEGAME_FILENAME);
    fin = fopen(buf, "rb");

    if (fin == NULL)
    {
        DUH_WHERE_AM_I("encountered a problem reading savegame.");
        return;
    }

    unlockables[UNLOCKS_EASY]   = (char)fgetc(fin);
    unlockables[UNLOCKS_MEDIUM] = (char)fgetc(fin);
    unlockables[UNLOCKS_HARD]   = (char)fgetc(fin);

    fclose(fin);
}

/**************************************************************************************************/
/*!
 * @brief Save a file storing what's unlocked already.
 */
void UNLOCKED_save(void)
{
    char buf[HOME_PATH_LENGTH + 19];
    FILE *fout;

    snprintf(buf, sizeof(buf) - 1, "%s/%s", common_user_home_path, SAVEGAME_FILENAME);
    fout = fopen(buf, "w+b");

    if (fout == NULL)
    {
        DUH_WHERE_AM_I("encountered a problem writing savegame.");
        return;
    }

    fseek(fout, 0, 0);

    fputc(unlockables[UNLOCKS_EASY  ], fout);
    fputc(unlockables[UNLOCKS_MEDIUM], fout);
    fputc(unlockables[UNLOCKS_HARD  ], fout);
    fclose(fout);
}
