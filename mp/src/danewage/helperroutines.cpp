#include "cbase.h"

#include "../game/client/clientsteamcontext.h"
#include "helperroutines.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool bMyVariable = false;


bool Increment_achievement_int(const char* someName, const bool clearAchievement)
{
	// STORMYS ACHIEVEMENT MESS
	// Create a new variable to hold our current data in
	int data;
	// Make the SYNCHRONOUS call to the API to get our current value.
	steamapicontext->SteamUserStats()->GetStat(someName, &data);
	// Make the SYNCHRONOUS call to the API to tell it our new value.  (It doesn't get uploaded yet.)
	steamapicontext->SteamUserStats()->SetStat(someName, data + 1);
	// Sometime later, we should save all our stats.  This uploads them to the server.
	steamapicontext->SteamUserStats()->StoreStats();
	if (clearAchievement)
		steamapicontext->SteamUserStats()->ResetAllStats(clearAchievement);
	// STORMYS ACHIEVEMENT MESS

	return true;
}

bool Achieve(const char* someName, const bool clearAchievement)
{
	bool data;
	steamapicontext->SteamUserStats()->GetAchievement(someName, &data);
	if (!data)
		steamapicontext->SteamUserStats()->SetAchievement(someName);
	if (clearAchievement)
		steamapicontext->SteamUserStats()->ClearAchievement(someName);
	return true;

}

bool GetMyVariable() { return bMyVariable; }
