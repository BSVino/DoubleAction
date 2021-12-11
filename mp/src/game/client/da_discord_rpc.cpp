//=============================================================================
//
// Purpose: Discord Rich Presence support.
//
//=============================================================================


#include "cbase.h"
#include "da_discord_rpc.h"
#include <ctime>
#include "strtools.h"
#include "Color.h"
#include "discord-rpc/discord_rpc.h"
#include "discord-rpc/discord_register.h"
#include "tier0/icommandline.h"
#include "ilocalize.h"
#include <stdlib.h>
#include "hl2_gamerules.h"
#include "c_playerresource.h"
#include <inetchannelinfo.h>
#include "c_hl2_playerlocaldata.h"
#include "c_baseplayer.h"
#include "filesystem.h"
#include "tier0/icommandline.h"
#include <vgui/ILocalize.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include <stdlib.h>
#include "igameevents.h"
#include "c_sdk_player.h"
#include "valve_minmax_on.h"
#include "valve_minmax_off.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

#define DISCORD_APP_ID "919072804941475881"

//using this instead of the old method for getting a timestamp so that the timestamp starts at 0
//when we join a server
//-Nbc66
time_t timestamp = NULL;

CTFDiscordRPC g_discordrpc;

CTFDiscordRPC::CTFDiscordRPC()
{
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	m_bInitializeRequested = false;
}

CTFDiscordRPC::~CTFDiscordRPC()
{
}

void CTFDiscordRPC::OnJoinGame(const char* joinSecret)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Game: %s\n", joinSecret);

	char szCommand[128];
	Q_snprintf(szCommand, sizeof(szCommand), "connect %s\n", joinSecret);
	engine->ExecuteClientCmd(szCommand);
}

void CTFDiscordRPC::OnJoinRequest(const DiscordUser* joinRequest)
{
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request: %s#%s\n", joinRequest->username, joinRequest->discriminator);
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Join Request Accepted\n");
	Discord_Respond(joinRequest->userId, DISCORD_REPLY_YES);
}

//stolen from KaydemonLP
//-Nbc66
const char* CTFDiscordRPC::GetRPCMapImage(char m_szLatchedMapname[MAX_MAP_NAME], const char* pMapIcon)
{
	KeyValues* pDiscordRPC = new KeyValues("Discord");
	pDiscordRPC->LoadFromFile(filesystem, "scripts/discord_rpc.txt");
	if (pDiscordRPC)
	{
		KeyValues* pMaps = pDiscordRPC->FindKey("Maps");
		if (pMaps)
		{
			return pMaps->GetString(m_szLatchedMapname, pMapIcon);
		}
		pMaps->deleteThis();
		pDiscordRPC->deleteThis();
	}
	return "da_icon";
}

const char* CTFDiscordRPC::GetPlayerModel()
{
	
	return "";
}


void CTFDiscordRPC::Shutdown()
{
	Discord_Shutdown();
}

void CTFDiscordRPC::InitializeDiscord()
{
	DiscordEventHandlers handlers;
	Q_memset(&handlers, 0, sizeof(handlers));
	handlers.ready = &CTFDiscordRPC::OnReady;
	handlers.errored = &CTFDiscordRPC::OnDiscordError;
	handlers.joinGame = &CTFDiscordRPC::OnJoinGame;
	//handlers.spectateGame = &CTFDiscordRPC::OnSpectateGame;
	handlers.joinRequest = &CTFDiscordRPC::OnJoinRequest;


	char command[512];
	V_snprintf(command, sizeof(command), "%s -game \"%s\" -novid -steam\n", CommandLine()->GetParm(0), CommandLine()->ParmValue("-game"));
	Discord_Register(DISCORD_APP_ID, command);
	Discord_Initialize(DISCORD_APP_ID, &handlers, false, "");
	Reset();
}


void CTFDiscordRPC::Init()
{
	InitializeDiscord();
	m_bInitializeRequested = true;

	// make sure to call this after game system initialized
	ListenForGameEvent("server_spawn");
}

void CTFDiscordRPC::RunFrame()
{
	if (m_bErrored)
		return;
	// NOTE: we want to run this even if they have use_discord off, so we can clear
	// any previous state that may have already been sent
	UpdateRichPresence();
	Discord_RunCallbacks();

	// always run this, otherwise we will chicken & egg waiting for ready
	//if (Discord_RunCallbacks)
	//	Discord_RunCallbacks();
}

void CTFDiscordRPC::OnReady(const DiscordUser* user)
{

	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] Ready!\n");
	ConColorMsg(Color(114, 137, 218, 255), "[Rich Presence] User %s#%s - %s\n", user->username, user->discriminator, user->userId);

	g_discordrpc.Reset();

}

void CTFDiscordRPC::OnDiscordError(int errorCode, const char* szMessage)
{
	char buff[1024];
	Q_snprintf(buff, 1024, "[Rich Presence] Init failed. code %d - error: %s\n", errorCode, szMessage);
	Warning("%s", buff);

	// ignore 4000 until I figure out what exactly is causing it
	if (errorCode != 4000)
		g_discordrpc.m_bErrored = true;

}

void CTFDiscordRPC::FireGameEvent(IGameEvent* event)
{
	const char* type = event->GetName();
	
	if (Q_strcmp(type, "server_spawn") == 0)
	{
		//setup the discord timestamp to be 0 when we join a server
		//-Nbc66
		timestamp = time(0);

		Q_strncpy(m_szLatchedHostname, event->GetString("hostname"), 255);
	}
}

void CTFDiscordRPC::SetLogo(void)
{

	const char* pszGameType = "ALL OUT ACTION!";
	const char* pszImageLarge = "da_icon";
	const char* pMapIcon = "da_icon";

	//string for setting the picture of the class
	//you should name the small picture affter the class itself ex: Scout.jpg, Soldier.jpg, Pyro.jpg ...
	//you get it
	//-Nbc66
	const char* pszImageSmall = "";
	const char* pszImageText = "";

	if (engine->IsConnected())
	{

		C_SDKPlayer* pPlayer = ToSDKPlayer(C_BasePlayer::GetLocalPlayer());

		if (pPlayer)
		{
			//show us a small image of the player character
			//-Nbc66
			const char* character = pPlayer->GetCharacter();
			DevMsg("current char is %s\n", character);
			//const char* smallimage = VarArgs("%s_playermodel", currentchartater);
			char formatedtext[256];
			//this is a formated string which will show us the name of the character and the total style
			//that we have when you hover over the small image
			//-Nbc66
			V_snprintf(formatedtext,sizeof(formatedtext),"%s : %i %s ", LocalizeDiscordString(VarArgs("#DA_Character_%s", character)) , RoundFloatToInt(pPlayer->GetStyleStars()), LocalizeDiscordString("#DA_PlayerScore"));
			pszImageText = formatedtext;
			const char* lowername = character;
			//used to set the small Image
			//-Nbc66
			pszImageSmall = lowername;
		}

		if (pszImageLarge != m_szLatchedMapname)
		{
			//stolen from KaydemonLP
			//-Nbc66
			pMapIcon = GetRPCMapImage(m_szLatchedMapname, pMapIcon);


			//old function that uses hard coded maps inside this cpp file
			//-Nbc66
			/*
					if (pszImageLarge != m_szLatchedMapname)
					{
						for (int i = 0; i < MAP_COUNT; i++)
						{
							if (V_strcmp(g_aMapList[i], m_szLatchedMapname) == 0)
							{
								pMapIcon = m_szLatchedMapname;
								break;
							}
						}
					}
			*/



			pszImageLarge = pMapIcon;

			m_sDiscordRichPresence.largeImageKey = pszImageLarge;
			m_sDiscordRichPresence.largeImageText = pszGameType;
			m_sDiscordRichPresence.smallImageKey = pszImageSmall;
			m_sDiscordRichPresence.smallImageText = pszImageText;

		}

	}
	m_sDiscordRichPresence.largeImageKey = pszImageLarge;
	m_sDiscordRichPresence.largeImageText = pszGameType;
}

void CTFDiscordRPC::UpdatePlayerInfo()
{
	C_PlayerResource* pResource = GetPlayerResource();
	if (!pResource)
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;

	const char* pzePlayerName = NULL;

	for (int i = 1; i < maxPlayers; i++)
	{
		if (pResource->IsConnected(i))
		{
			curPlayers++;
			if (pResource->IsLocalPlayer(i))
			{
				pzePlayerName = pResource->GetPlayerName(i);
			}
		}
	}

	//int iTimeLeft = TFGameRules()->GetTimeLeft();

	if (m_szLatchedHostname[0] != '\0')
	{
		m_sDiscordRichPresence.partySize = curPlayers;
		m_sDiscordRichPresence.partyMax = maxPlayers;
		m_sDiscordRichPresence.state = m_szLatchedHostname;
		//m_sDiscordRichPresence.state = szStateBuffer;
	}
}

void CTFDiscordRPC::UpdateNetworkInfo()
{
	INetChannelInfo* ni = engine->GetNetChannelInfo();
	if (ni)
	{
		// adding -party here because secrets cannot match the party id
		m_sDiscordRichPresence.partyId = VarArgs("%s-party", ni->GetAddress());
		m_sDiscordRichPresence.joinSecret = ni->GetAddress();
	}

	//dosent work untill i can figgure out how to get the source tv ip
	//m_sDiscordRichPresence.spectateSecret = "Spectate";
}



void CTFDiscordRPC::LevelInit(const char* szMapname)
{
	Reset();
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, szMapname);
	//V_snprintf(szStateBuffer, sizeof(szStateBuffer), "MAP: %s", m_szLatchedMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = MAX(0, gpGlobals->realtime - DISCORD_UPDATE_RATE);
}


//Function to get the localized string and then localize it on runtime
//Use this to localize the rest of the strings
//
//-Nbc66
const char* CTFDiscordRPC::LocalizeDiscordString(const char* LocalizedString)
{

	const wchar_t* WcharLocalizedString = g_pVGuiLocalize->Find(LocalizedString);
	//char array is set this way to account for ASCII's
	//characters which are generaly 256 charachetrs with Windows-1252 8-bit charachter encoding
	//just dont fuck with the array size or you are going to have a bad time man
	//-Nbc66
	char CharLocalizedArray[256];
	g_pVGuiLocalize->ConvertUnicodeToANSI(WcharLocalizedString, CharLocalizedArray, sizeof(CharLocalizedArray));
	const char* FinalCharLocalizedString = V_strdup(CharLocalizedArray);

	return FinalCharLocalizedString;

	delete[] FinalCharLocalizedString;
}

void CTFDiscordRPC::UpdateRichPresence()
{

	if (!NeedToUpdate())
		return;

	m_flLastUpdatedTime = gpGlobals->realtime;

	if (engine->IsConnected())
	{
		V_snprintf(szStateBuffer, sizeof(szStateBuffer), "%s : %s", LocalizeDiscordString("#Discord_Map"), m_szLatchedMapname);
		//starts the elapsed timer for discord rpc
		//-Nbc66
		m_sDiscordRichPresence.startTimestamp = timestamp;
		g_discordrpc.UpdatePlayerInfo();
		g_discordrpc.UpdateNetworkInfo();
		//sets the map name
		m_sDiscordRichPresence.details = szStateBuffer;
	}

	//checks if the loading bar is being drawn
	//and sets the discord status to "Currently is loading..."
	//-Nbc66
	if (engine->IsDrawingLoadingImage() == true)
	{
		m_sDiscordRichPresence.state = "";
		m_sDiscordRichPresence.details = "Currently loading...";
	}

	SetLogo();

	Discord_UpdatePresence(&m_sDiscordRichPresence);
}

bool CTFDiscordRPC::NeedToUpdate()
{
	if (m_bErrored || m_szLatchedMapname[0] == '\0')
		return false;

	return gpGlobals->realtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

void CTFDiscordRPC::Reset()
{
	Q_memset(&m_sDiscordRichPresence, 0, sizeof(m_sDiscordRichPresence));
	m_sDiscordRichPresence.details = LocalizeDiscordString("#Discord_State_Menu");
	const char* pszState = "";

	m_sDiscordRichPresence.state = pszState;

	m_sDiscordRichPresence.endTimestamp;

	SetLogo();
	Discord_UpdatePresence(&m_sDiscordRichPresence);
}

