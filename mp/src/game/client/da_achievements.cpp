//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Achievements for DA
//		these achievements are triggered when the server fires relevant events
//
//=============================================================================



//#ifdef GAME_DLL


#include "cbase.h"

#include "baseachievement.h"
#include "achievementmgr.h"
#include "sdk_player_shared.h"
#include "c_sdk_player.h"

CAchievementMgr* g_pAchivementManager = new CAchievementMgr();


// GET A DIVEPUNCH KILL
class CAchievementDAdivepunch1 : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEPUNCHKILL");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEPUNCHKILL") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define DIVEPUNCHKILL 4 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAdivepunch1, DIVEPUNCHKILL, "DIVEPUNCHKILL", 1)



// GET 250 DIVEPUNCH KILLS
class CAchievementDAdivepunch250 : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(250);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEPUNCHKILL");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEPUNCHKILL") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DIVEPUNCHKILL_250 7
DECLARE_ACHIEVEMENT(CAchievementDAdivepunch250, DIVEPUNCHKILL_250, "DIVEPUNCHKILL_250", 1)



// GET 1000 DIVEPUNCH KILLS
class CAchievementDAdivepunch1000 : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEPUNCHKILL");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEPUNCHKILL") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DIVEPUNCHKILL_1000 22
DECLARE_ACHIEVEMENT(CAchievementDAdivepunch1000, DIVEPUNCHKILL_1000, "DIVEPUNCHKILL_1000", 1)



// GET like a bajillion DIVEPUNCH KILLS
class CAchievementDAdivepunchBajillion : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEPUNCHKILL");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEPUNCHKILL") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DIVEPUNCHKILL_BAJILLION 8
DECLARE_ACHIEVEMENT(CAchievementDAdivepunchBajillion, DIVEPUNCHKILL_BAJILLION, "DIVEPUNCHKILL_BAJILLION", 1)







// duvepunchkill while superfalling
class CAchievementDASkypunch : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("SKYPUNCH");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "SKYPUNCH") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define SKYPUNCH 14
DECLARE_ACHIEVEMENT(CAchievementDASkypunch, SKYPUNCH, "SKYPUNCH", 1)






// Dive away from an explosion for reduced damage
class CAchievementDAdiveaweayfromexplosion : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEAWAYFROMEXPLOSION");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEAWAYFROMEXPLOSION") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			// if the userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DIVEAWAYFROMEXPLOSION 10
DECLARE_ACHIEVEMENT(CAchievementDAdiveaweayfromexplosion, DIVEAWAYFROMEXPLOSION, "DIVEAWAYFROMEXPLOSION", 1)







// Dive away from an explosion for reduced damage 250 times
class CAchievementDAdiveaweayfromexplosion250 : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(250);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DIVEAWAYFROMEXPLOSION_250");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DIVEAWAYFROMEXPLOSION_250") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			// if the userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DIVEAWAYFROMEXPLOSION_250 5
DECLARE_ACHIEVEMENT(CAchievementDAdiveaweayfromexplosion250, DIVEAWAYFROMEXPLOSION_250, "DIVEAWAYFROMEXPLOSION_250", 1)






// Point blank head shot
class CAchievementDAdodgethis : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DODGETHIS");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DODGETHIS") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			// if the userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};
#define DODGETHIS 11
DECLARE_ACHIEVEMENT(CAchievementDAdodgethis, DODGETHIS, "DODGETHIS", 1)





// Kill a player with their own weapon
class CAchievementDABetrayed : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("BETRAYED");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "BETRAYED") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define BETRAYED 20 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDABetrayed, BETRAYED, "BETRAYED", 1)





// Kill a player with a headshot after backflipping off them
class CAchievementDAHardboiled : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("HARDBOILED");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "HARDBOILED") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define HARDBOILED 21 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAHardboiled, HARDBOILED, "HARDBOILED", 1)





// 250 KILLSTREAK
class CAchievementDAPenguin : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("PENGUIN");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "PENGUIN") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define PENGUIN 12 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAPenguin, PENGUIN, "PENGUIN", 1)






/**************************************************************
	Fall Guy, Plunger and Deep Impact all use the same game event - FALL_GUY
*/
// Fall Guy - 50 superfall kills
class CAchievementDAFallguy : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(50);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("FALL_GUY");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "FALL_GUY") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define FALL_GUY 24 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAFallguy, FALL_GUY, "FALL_GUY", 1)

// Plunger - 250 superfall kills
class CAchievementDAPlunger : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(250);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("FALL_GUY");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "FALL_GUY") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define PLUNGER 25 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAPlunger, PLUNGER, "PLUNGER", 1)

// Deep Impact - 1000 superfall kills
class CAchievementDADeepImpact : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("FALL_GUY");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "FALL_GUY") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define DEEP_IMPACT 26 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDADeepImpact, DEEP_IMPACT, "DEEP_IMPACT", 1)






// SUPERFALL_SHARPSHOOTER - 3 kills in a single superfall
class CAchievementDASuperfallSharpshooter : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("SUPERFALL_SHARPSHOOTER");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "SUPERFALL_SHARPSHOOTER") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define SUPERFALL_SHARPSHOOTER 27 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDASuperfallSharpshooter, SUPERFALL_SHARPSHOOTER, "SUPERFALL_SHARPSHOOTER", 1)


// SUPERFALL_KING - 5 kills in a single superfall
class CAchievementDASuperfallKing : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("SUPERFALL_KING");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "SUPERFALL_KING") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define SUPERFALL_KING 28 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDASuperfallKing, SUPERFALL_KING, "SUPERFALL_KING", 1)


// HINDU_COWS - 3 kills in a single superfall, all headshots, with a pistol
class CAchievementDAHinduCows : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("HINDU_COWS");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "HINDU_COWS") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define HINDU_COWS 29 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAHinduCows, HINDU_COWS, "HINDU_COWS", 1)










/************************************************
	Weapon grind achievements
*/

class CAchievementDAUndertaken : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("UNDERTAKEN");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "UNDERTAKEN") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define UNDERTAKEN 30 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAUndertaken, UNDERTAKEN, "UNDERTAKEN", 1)


class CAchievementDAVindicated : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("VINDICATED");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "VINDICATED") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define VINDICATED 31 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAVindicated, VINDICATED, "VINDICATED", 1)


class CAchievementDAPersuaded : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("PERSUADED");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "PERSUADED") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define PERSUADED 32 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAPersuaded, PERSUADED, "PERSUADED", 1)


class CAchievementDAMacDaddyd : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("MAC_DADDYD");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "MAC_DADDYD") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define MAC_DADDYD 33 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAMacDaddyd, MAC_DADDYD, "MAC_DADDYD", 1)


class CAchievementDAHorseWhisperer : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("HORSE_WHISPERER");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "HORSE_WHISPERER") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define HORSE_WHISPERER 34 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAHorseWhisperer, HORSE_WHISPERER, "HORSE_WHISPERER", 1)


class CAchievementDAVigilant : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("VIGILANT");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "VIGILANT") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define VIGILANT 35 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAVigilant, VIGILANT, "VIGILANT", 1)


class CAchievementDABlackMagicked : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(10000);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("BLACK_MAGICKED");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "BLACK_MAGICKED") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define BLACK_MAGICKED 36 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDABlackMagicked, BLACK_MAGICKED, "BLACK_MAGICKED", 1)

/*
	end Weapon grind achievements
************************************************/




// POTSHOTTER - long range kill using pistol on rifle wielding victim
class CAchievementDAPotshotter : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("POTSHOTTER");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "POTSHOTTER") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define POTSHOTTER 17 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDAPotshotter, POTSHOTTER, "POTSHOTTER", 1)




// SLOWPRO - 3 kills in one slowmo
class CAchievementDASlowpro : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("SLOWPRO");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "SLOWPRO") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define SLOWPRO 15 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDASlowpro, SLOWPRO, "SLOWPRO", 1)




// SPECIAL_DELIVERY - capture briefcase without killing anyone
class CAchievementDASpecialDelivery : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("SPECIALDELIVERY");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "SPECIALDELIVERY") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define SPECIALDELIVERY 15 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDASpecialDelivery, SPECIALDELIVERY, "SPECIALDELIVERY", 1)




// NO_YOU_DONT - kill a player who has their slowmo activated
class CAchievementDANoYouDont : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(1);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("NO_YOU_DONT");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "NO_YOU_DONT") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(); // WE ALL GOOD!
			}
		}
	}
};

#define NO_YOU_DONT 18 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDANoYouDont, NO_YOU_DONT, "NO_YOU_DONT", 1)


//#endif // GAME_DLL