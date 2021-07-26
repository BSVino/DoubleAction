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



// GET 250 DIVEPUNCH KILLS
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
		SetGoal(651438);
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





// 250 KILLSTREAK
class CAchievementDADicoStu : public CBaseAchievement
{
protected:
	virtual void Init()
	{
		SetFlags(ACH_LISTEN_PLAYER_KILL_ENEMY_EVENTS | ACH_SAVE_GLOBAL);
		SetGoal(100);
		m_bStoreProgressInSteam = true;
	}

	// register our event listeners
	virtual void ListenForEvents()
	{
		ListenForGameEvent("DISCOSTU");
	}

	// define what happens when we catch an event
	void FireGameEvent_Internal(IGameEvent *event)
	{
		// compare event names and check that we have a local player
		if (0 == Q_strcmp(event->GetName(), "DISCOSTU") && C_BasePlayer::GetLocalPlayer())
		{
			int iUserID = event->GetInt("userid"); // the userID passed from the event data
			float amount = event->GetInt("amount"); // the amount to add to the stat

			DevMsg("\n increasing DISCOSTU by $f", amount);

			// if the atackers userID from the event matches the local player
			if (iUserID == C_BasePlayer::GetLocalPlayer()->GetUserID())
			{
				IncrementCount(amount); // WE ALL GOOD!
			}
		}
	}
};

#define DISCOSTU 13 // the stat ID and name from steamworks - not the achievement ID
DECLARE_ACHIEVEMENT(CAchievementDADicoStu, DISCOSTU, "DISCOSTU", 1)


//#endif // GAME_DLL