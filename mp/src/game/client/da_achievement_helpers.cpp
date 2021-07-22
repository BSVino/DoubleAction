//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Achievements for DA
//		these achievements are triggered when the server fires relevant events
//
//=============================================================================



//#ifdef GAME_DLL

#include "cbase.h"

#include "c_baseplayer.h"

#include "clientsteamcontext.h"
#include "da_achievement_helpers.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool DA_InitStats(){
	return steamapicontext->SteamUserStats()->RequestCurrentStats();
}

bool DA_IncrementStat(const char* name, C_BasePlayer* pPlayer, const int num, const bool clearAchievement)
{
	if (pPlayer != C_BasePlayer::GetLocalPlayer()){
		DevMsg("attempted to increment stat for non-local player\n");
		return false; // only increment stats for the local player
	}
	else {
		DevMsg("incrementing stat for player\n");
	}
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

bool DA_AwardAchievement(const char* name, C_BasePlayer* pPlayer, const bool clearAchievement)
{
	if (pPlayer != C_BasePlayer::GetLocalPlayer()){
		DevMsg("attempted to award achievement for non-local player\n");
		return false; // only award achievements for the local player
	}
	else {
		DevMsg("awarding achievement for player\n");
	}

	// only the server can award achievements, so send a message to the server to get it done
	// create a game event usnig the passed name
	IGameEvent * event = gameeventmanager->CreateEvent(name);
	if (event == NULL){
		DevMsg("event %s not registered - has it been added to modevents.res?", name);
		return false;
	}
	event->SetInt("userid", pPlayer->GetUserID()); // we're assuming all of our achievement events have user ids - the user to whom we will award the achievement
	gameeventmanager->FireEvent(event);

	/*
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
	*/

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
