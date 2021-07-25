//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Achievement helper functions for DA
//		wraps firing achievement events to the client
//
//=============================================================================



//#ifdef GAME_DLL

#include "cbase.h"
#include "sdk_player.h"
#include "da_achievement_helpers.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



// this code only runs on the server and our achievements are implemented on the client
// so we just want to fire off an event that the achievement on the client is listening to
void DA_ApproachAchievement(const char* achievement_event_name, CSDKPlayer* pPlayer){

	DevMsg("\n\nevent %s requested\n\n\n", achievement_event_name);
	// create our achievement event
	IGameEvent* event = gameeventmanager->CreateEvent(achievement_event_name);
	if (event)
	{
		DevMsg("\n\nevent %s created\n\n\n", achievement_event_name);
		// set our event attacker userID to send to the client
		event->SetInt("userid", pPlayer->GetUserID());
		// send it off
		gameeventmanager->FireEvent(event);
	}
}
