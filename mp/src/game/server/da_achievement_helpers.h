
#ifndef DAHELPERS
#define DAHELPERS
bool DA_InitStats();
bool DA_ClearAllStats();
void DA_PrintStat(const char* name);
bool DA_IncrementStat(const char* name, const int num = 1, const bool clearAchievement = false);
bool DA_AwardAchievement(const char* name, const bool clearAchievement = false);
void DA_PrintAchievementStatus(const char* name);
void DA_PrintAchievementProgress(const char* name, const uint32 num, const uint32 max);


#define DIVEPUNCHKILL 4 // the stat ID and name from steamworks - not the achievement ID
#define DIVEPUNCHKILL_250 7
#define DIVEPUNCHKILL_BAJILLION 8
#define DIVEAWAYFROMEXPLOSION 10
#define DIVEAWAYFROMEXPLOSION_250 5
#define DODGETHIS 11

#endif