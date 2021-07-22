
#ifndef DAHELPERS
#define DAHELPERS
bool DA_InitStats();
void DA_PrintStat(const char* name);
bool DA_IncrementStat(const char* name, C_BasePlayer* pPlayer, const int num = 1, const bool clearAchievement = false);
bool DA_AwardAchievement(const char* name, C_BasePlayer* pPlayer, const bool clearAchievement = false);
void DA_PrintAchievementStatus(const char* name);


#define DIVEPUNCHKILL 4 // the stat ID and name from steamworks - not the achievement ID
#define DIVEPUNCHKILL_250 7
#define DIVEPUNCHKILL_BAJILLION 8
#define DIVEAWAYFROMEXPLOSION 10
#define DIVEAWAYFROMEXPLOSION_250 5
#define DODGETHIS 11

#endif