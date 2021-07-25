//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "da_achievementlistener.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DA_AchievementListener::DA_AchievementListener()
{
}

DA_AchievementListener::~DA_AchievementListener()
{
}

void DA_AchievementListener::FireGameEvent(IGameEvent *event)
{
	HandleEvent(event);
}

void DA_AchievementListener::HandleEvent(IGameEvent *event){
	const char* name = event->GetName();
	Warning("DA_AchievementListener::HandleEvent - %s", name);
}


bool DA_AchievementListener::Init()
{
	Warning("MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
	ListenForGameEvent("DIVEPUNCHKILL");
	ListenForGameEvent("DIVEPUNCHKILL_250");
	ListenForGameEvent("DIVEPUNCHKILL_BAJILLION");
	ListenForGameEvent("DIVEAWAYFROMEXPLOSION");
	ListenForGameEvent("DIVEAWAYFROMEXPLOSION_250");
	ListenForGameEvent("DODGETHIS");

	return true;
}

void DA_AchievementListener::Shutdown()
{
	StopListeningForAllEvents();
}