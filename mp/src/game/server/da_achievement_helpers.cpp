//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Achievements for DA
//		these achievements are triggered when the server fires relevant events
//
//=============================================================================



//#ifdef GAME_DLL

#include "cbase.h"

#include "sdk_player.h"

#include "../client/clientsteamcontext.h"
#include "da_achievement_helpers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool DA_InitStats(){
	return steamapicontext->SteamUserStats()->RequestCurrentStats();
}

bool DA_ClearAllStats(){
	return steamapicontext->SteamUserStats()->RequestCurrentStats();
}

bool DA_IncrementStat(const char* name, const int num, const bool clearAchievement)
{
	// Create a new variable to hold our current data in
	int data;
	// Make the SYNCHRONOUS call to the API to get our current value.
	steamapicontext->SteamUserStats()->GetStat(name, &data);
	// Make the SYNCHRONOUS call to the API to tell it our new value.  (It doesn't get uploaded yet.)
	steamapicontext->SteamUserStats()->SetStat(name, data + num);
	DA_PrintStat(name);

	// Sometime later, we should save all our stats.  This uploads them to the server.
	steamapicontext->SteamUserStats()->StoreStats();
	if (clearAchievement)
		steamapicontext->SteamUserStats()->ResetAllStats(clearAchievement);

	return true;
}

void DA_PrintStat(const char* name){
	// Create a new variable to hold our current data in
	int data;
	// Make the SYNCHRONOUS call to the API to get our current value.
	steamapicontext->SteamUserStats()->GetStat(name, &data);
	// print it
	DevMsg("stat %s: %i\n", name, data);
}

bool DA_AwardAchievement(const char* name, const bool clearAchievement)
{
	bool achieved;
	steamapicontext->SteamUserStats()->GetAchievement(name, &achieved);
	if (!achieved){
		steamapicontext->SteamUserStats()->SetAchievement(name);
		steamapicontext->SteamUserStats()->StoreStats();
	}
	if (clearAchievement){
		steamapicontext->SteamUserStats()->ClearAchievement(name);
		steamapicontext->SteamUserStats()->StoreStats();
	}

	DA_PrintAchievementStatus(name);

	return true;

}


void DA_PrintAchievementStatus(const char* name){
	// Create a new variable to hold our current data in
	bool achieved;
	// Make the SYNCHRONOUS call to the API to get our current value.
	steamapicontext->SteamUserStats()->GetAchievement(name, &achieved);
	// print it
	if (achieved){
		DevMsg("achievement %s: achieved\n", name);
	}
	else {
		DevMsg("achievement %s: NOT achieved\n", name);
	}
}

void DA_PrintAchievementProgress(const char* name, const uint32 num, const uint32 max){
	steamapicontext->SteamUserStats()->IndicateAchievementProgress(name, num, max);
}
