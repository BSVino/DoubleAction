//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Achievement helper functions for DA
//		wraps firing achievement events to the client
//
//=============================================================================



//#ifdef GAME_DLL

#include "cbase.h"
#include "da_achievement_helpers.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



// this code only runs on the server and our achievements are implemented on the client
// so we just want to fire off an event that the achievement on the client is listening to
void DA_ApproachAchievement(const char* achievement_event_name, int player_id, int amount){

	DevMsg("\n\nevent %s requested from server\n\n\n", achievement_event_name);
	// create our achievement event
	IGameEvent* event = gameeventmanager->CreateEvent(achievement_event_name);
	if (event)
	{
		DevMsg("\n\nevent %s created from server\n\n\n", achievement_event_name);
		// set our event attacker userID to send to the client
		event->SetInt("userid", player_id);
		event->SetInt("amount", amount);
		if (amount > 1){
			DevMsg("\n\nevent %s increasing by %i\n\n\n", achievement_event_name, amount);
		}
		
		// send it off
		gameeventmanager->FireEvent(event);
	}
}
