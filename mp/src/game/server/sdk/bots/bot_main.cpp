//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#include "cbase.h"
#include "player.h"
#include "sdk_player.h"

#include "in_buttons.h"
#include "gameinterface.h"

#include "bot_main.h"
#include "sdk_bot.h"
#include "sdk_gamerules.h"

// support for nav mesh
#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CSDKBot;
void Bot_Think( CSDKBot *pBot );


// Handler for the "bot" command.
CON_COMMAND_F( bot_add, "Add a bot.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if( !TheNavMesh->IsLoaded() )
		Warning( "No navigation mesh loaded! Can't create bot" );

	// Look at -count.
	int count = args.FindArgInt( "-count", 1 );
	count = clamp( count, 1, 16 );

	// Look at -frozen.
	bool bFrozen = !!args.FindArg( "-frozen" );
		
	// Ok, spawn all the bots.
	while ( --count >= 0 )
	{
		BotPutInServer( bFrozen );
	}
}

CON_COMMAND_F( bot_kick, "Kick all bots.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	ConVarRef bot_quota("bot_quota");
	bot_quota.SetValue(0);
}

static int g_CurBotNumber = 1;


class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CSDKBot *pPlayer = static_cast<CSDKBot *>( CreateEntityByName( "bot" ) );
		if ( pPlayer )
		{
			pPlayer->SetPlayerName( playername );
		}

		return pPlayer;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool  bFrozen )
{
	char botname[ 64 ];
	Q_snprintf( botname, sizeof( botname ), "Bot%02i", g_CurBotNumber );
	
	// This trick lets us create a CSDKBot for this client instead of the CSDKPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( botname );
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Msg( "Failed to create Bot (no edict available)\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CSDKBot *pPlayer = ((CSDKBot*)CBaseEntity::Instance( pEdict ));

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	pPlayer->ChangeTeam( 0 );
	pPlayer->RemoveAllItems( true );

	pPlayer->Initialize();

	g_CurBotNumber++;

	return pPlayer;
}

static ConVar bot_quota("bot_quota", "0", FCVAR_ARCHIVE|FCVAR_GAMEDLL, "Try to keep this many bots in the server");

void Bot_MaintainQuota()
{
	int iIdealNumberOfBots = bot_quota.GetInt();

	int iCurrentHumans = 0;
	int iCurrentBots = 0;

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (pPlayer->IsBot())
			iCurrentBots++;
		else
			iCurrentHumans++;
	}

	if (!engine->IsDedicatedServer() && iCurrentHumans == 0)
		return;

	if ( !SDKGameRules() )
		return;

	if (iCurrentHumans == 0)
		iIdealNumberOfBots = 0;

	if (iCurrentHumans + iIdealNumberOfBots >= gpGlobals->maxClients - 1)
		iIdealNumberOfBots = gpGlobals->maxClients - iCurrentHumans - 1;

	if (iCurrentBots == iIdealNumberOfBots)
		return;

	while (iCurrentBots < iIdealNumberOfBots)
	{
		BotPutInServer( false );
		iCurrentBots++;
	}

	while (iCurrentBots > iIdealNumberOfBots)
	{
		CSDKPlayer* pBotToKick = NULL;
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

			if (!pPlayer)
				continue;

			if (pPlayer->IsBot())
			{
				pBotToKick = pPlayer;
				break;
			}
		}

		Assert(pBotToKick);
		if (pBotToKick)
		{
			engine->ServerCommand(UTIL_VarArgs( "kick \"%s\"\n", pBotToKick->GetPlayerName() ));
			iCurrentBots--;
		}
		else
			break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{	
	Bot_MaintainQuota();

	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		if (!pPlayer)
			continue;

		if (!pPlayer->IsBot())
			continue;

		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CSDKBot *pBot = static_cast< CSDKBot* >( pPlayer );
			pBot->BotThink();
		}
	}
}
