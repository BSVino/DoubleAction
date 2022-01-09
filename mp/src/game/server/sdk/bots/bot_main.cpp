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
CON_COMMAND_F( bot_add, "Add a bot. Optionally takes a name for the bot, otherwise a random name will be chosen.", FCVAR_GAMEDLL )
{
	if ( !UTIL_IsCommandIssuedByServerAdmin() )
		return;

	if( !TheNavMesh->IsLoaded() )
		Warning( "No navigation mesh loaded! Can't create bot" );

	ConVarRef bot_quota("bot_quota");
	bot_quota.SetValue(bot_quota.GetInt() + 1);

	if (!FStrEq("", args.ArgS())) {
		BotPutInServer(false, args.ArgS());
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

const char* g_aszBotNames[] =
{
	"Brohan",
	"Brochill",
	"Broseph von Brohammer",
	"Brotastic Broski",
	"Brosicle",
	"Brofessor Brobody",
	"Han Brolo",
	"Broseidon, Lord of the Brocean",
	"Broba Fett",
	"Brohatma Ghandi",
	"Brohemian",
	"Bromosapien",
	"Broseph Stalin",
	"Abroham Lincoln",
	"Brorack Brobama",
	"Bro Biden",
	"Broranosaurus rex",
	"Brohemoth",
	"Broseph Gordon Levitt",
	"Brobi-wan Kenobi",
	"Marco Brolo",
	"Edgar Allan Bro",
	"Brozo the Clown",
	"C-3P Bro",
	"Frosty the Broman",
	"G.I. Bro",
	"Brose Marti",
	"The Higgs Broson",
	"Brodo Baggins",
	"Bilbro Baggins",
	"Teddy Broosevelt",
	"Franklin Broosevelt",
	"Broam Chomsky",
	"Brozilla",
	"Napoleon Bronaparte",
	"Brostradamos",
	"Quasibrodo",
	"Jon Bon Brovi",
	"Brobe Bryant",
	"Mr. Broboto",
	"Brolin Powell",
	"Brofi Annan",
	"Conan Bro'Brien",
	"Arnold Brozenegger",
	"Bro Yun Fat",
	"Pierce Brosnan",
	"Samuel Bro Jackson",
	"Quentin Broantino",
	"Clive Browen",
	"Elvis Brosely",
	"Demi Brovato",
	"Selena Bromez",
	"Michael Broson",
	"Ton Broosendaal",
	"Broctor Death",
	"Spiderbro",
	"Doctor Broctopus",
	"Bro Nye the Science Guy",
	"Bromethius",
	"Bromance",
	"Broland of Gilead",
	"Bro Jackson",
	"Indiana Brones",
	"Big Lebroski",
	"Angelina Broli",
	"Vincent van Bro",
	"Bromer Simpson",
	"Bromeo",
	"Kurt Brobain",
	"Broald Dahl",
	"Scarlett Brohansen",
	"Patty Bro'brien",
};


//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool  bFrozen, const char *name )
{
	int iNumBotNames = ARRAYSIZE(g_aszBotNames);

	// This trick lets us create a CSDKBot for this client instead of the CSDKPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	if (!name)
	{
		for (int i = 0; i < 5; ++i) {
			name = g_aszBotNames[random->RandomInt(0, iNumBotNames - 1)];
			if (!UTIL_PlayerByName(name))
				break;
		}
	}
	edict_t *pEdict = engine->CreateFakeClient(name);
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
static ConVar bot_reconnect("bot_reconnect", "0", FCVAR_CHEAT);

void Bot_MaintainQuota()
{
	int iIdealNumberOfBots = bot_quota.GetInt();

	static float last_reconnect = 0;
	if (bot_reconnect.GetBool())
	{
		if (gpGlobals->curtime > last_reconnect + bot_reconnect.GetFloat() || gpGlobals->curtime < last_reconnect)
		{
			if (bot_quota.GetInt() > 0)
			{
				RandomSeed(gpGlobals->framecount);
				int initial_player = RandomInt(1, gpGlobals->maxClients);
				int player = initial_player;

				while ((++player) % (gpGlobals->maxClients + 1) != initial_player)
				{
					CSDKPlayer* pPlayer = ToSDKPlayer(UTIL_PlayerByIndex(player % (gpGlobals->maxClients + 1)));
					if (!pPlayer)
						continue;

					if (!pPlayer->IsBot())
						continue;

					engine->ServerCommand(UTIL_VarArgs("kick \"%s\"\n", pPlayer->GetPlayerName()));
					break;
				}
			}

			last_reconnect = gpGlobals->curtime;
		}
	}

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

	if (engine->IsDedicatedServer())
		iIdealNumberOfBots = min(iIdealNumberOfBots, 4);

	if (iCurrentBots == iIdealNumberOfBots)
		return;

	RandomSeed(gpGlobals->framecount);
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

	random->SetSeed(gpGlobals->curtime*1000);
	RandomSeed(gpGlobals->curtime*1000);

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
