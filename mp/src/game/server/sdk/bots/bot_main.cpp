//******************************************************************
// Multiplayer AI for Source engine by R_Yell - rebel.y3ll@gmail.com
//******************************************************************

#include "cbase.h"
#include "player.h"
#include "sdk_player.h"

#include "in_buttons.h"
#include "movehelper_server.h"
#include "gameinterface.h"

#include "bot_main.h"
#include "bot_combat.h"
#include "bot_navigation.h"

// support for nav mesh
#include "nav_mesh.h"
#include "nav_pathfind.h"
#include "nav_area.h"

class CSDKBot;
void Bot_Think( CSDKBot *pBot );


// Handler for the "bot" command.
CON_COMMAND_F( bot_add, "Add a bot.", 0 /*FCVAR_CHEAT*/ )
{
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

static int g_CurBotNumber = 1;


LINK_ENTITY_TO_CLASS( bot, CSDKBot );


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

	// Spawn() doesn't work with MP template codebase, even if this line is part of default bot template...
	//pPlayer->Spawn();

	CCommand args;
	args.Tokenize( "joingame" );
	pPlayer->ClientCommand( args );

	// set bot skills
	pPlayer->m_flSkill[BOT_SKILL_YAW_RATE] = random->RandomFloat(SKILL_MIN_YAW_RATE, SKILL_MAX_YAW_RATE);
	pPlayer->m_flSkill[BOT_SKILL_SPEED] = random->RandomFloat(SKILL_MIN_SPEED, SKILL_MAX_SPEED);
	pPlayer->m_flSkill[BOT_SKILL_STRAFE] = random->RandomFloat( SKILL_MIN_STRAFE, SKILL_MAX_STRAFE);

	g_CurBotNumber++;

	return pPlayer;
}

//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{	
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CSDKPlayer *pPlayer = ToSDKPlayer( UTIL_PlayerByIndex( i ) );

		// Ignore plugin bots
		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CSDKBot *pBot = dynamic_cast< CSDKBot* >( pPlayer );
			if ( pBot )
			{
				Bot_Think( (CSDKBot *)pPlayer );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input  : *fakeclient - 
//			*viewangles - 
//			forwardmove - 
//			m_flSideMove - 
//			upmove - 
//			buttons - 
//			impulse - 
//			msec - 
// Output : 	virtual void
//-----------------------------------------------------------------------------
static void RunPlayerMove( CSDKPlayer *fakeclient, CUserCmd &cmd, float frametime )
{
	if ( !fakeclient )
		return;

	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	fakeclient->SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( fakeclient );
	fakeclient->PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	fakeclient->SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	fakeclient->pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}

void Bot_HandleRespawn( CSDKBot *pBot, CUserCmd &cmd )
{		
	// Try hitting my buttons occasionally
	if ( random->RandomInt( 0, 100 ) > 80 )
	{
		// Respawn the bot
		if ( random->RandomInt( 0, 1 ) == 0 )
		{
			cmd.buttons |= IN_JUMP;
		}
		else
		{
			cmd.buttons = 0;
		}
	}	
}

// here bot updates important info that is used multiple times along the thinking process
void BotInfoGathering( CSDKBot *pBot )
{	
	pBot->m_flBotToEnemyDist = (pBot->GetLocalOrigin() - pBot->GetEnemy()->GetLocalOrigin()).Length();

	trace_t tr;
	UTIL_TraceHull(  pBot->EyePosition(), pBot->GetEnemy()->EyePosition() - Vector(0,0,20), -BotTestHull, BotTestHull, MASK_SHOT, pBot, COLLISION_GROUP_NONE, &tr );
	
	if( tr.m_pEnt == pBot->GetEnemy() ) // vision line between both
		pBot->m_bEnemyOnSights = true;
	else
		pBot->m_bEnemyOnSights = false;

	pBot->m_bInRangeToAttack = (pBot->m_flBotToEnemyDist < pBot->m_flMinRangeAttack) && pBot->FInViewCone( pBot->GetEnemy() ); 

	pBot->m_flDistTraveled += fabs(pBot->GetLocalVelocity().Length()); // this is used for stuck checking, 

	pBot->m_flHeightDifToEnemy = pBot->GetLocalOrigin().z - pBot->GetEnemy()->GetLocalOrigin().z;
}
	

//-----------------------------------------------------------------------------
// Run this Bot's AI for one tick.
//-----------------------------------------------------------------------------
void Bot_Think( CSDKBot *pBot )
{
	// Make sure we stay being a bot
	pBot->AddFlag( FL_FAKECLIENT );

	if ( pBot->IsEFlagSet(EFL_BOT_FROZEN) )
		return;

	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );	

	if ( !pBot->IsAlive() )
	{
		Bot_HandleRespawn( pBot, cmd );
	}
	else
	{
		trace_t tr_front;
		Vector Forward;
		AngleVectors(pBot->GetLocalAngles(), &Forward);
		UTIL_TraceHull( pBot->GetLocalOrigin()+Vector(0,0,5), pBot->GetLocalOrigin() + Vector(0,0,5) + (Forward * 50), pBot->GetPlayerMins(), pBot->GetPlayerMaxs(), MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &tr_front );

		// enemy acquisition
		if( !pBot->GetEnemy() || pBot->RecheckEnemy() || !pBot->GetEnemy()->IsAlive() )
		{
			if( pBot->GetEnemy() && !pBot->GetEnemy()->IsAlive() )
				pBot->ResetNavigationParams();

			if( !AcquireEnemy( pBot ) )
				return;
	
			pBot->m_flTimeToRecheckEnemy = gpGlobals->curtime + 1.0f;
		}

		// assume we have an enemy from now on

		BotInfoGathering( pBot );

		BotAttack( pBot, cmd );

		if( pBot->m_flTimeToRecheckStuck < gpGlobals->curtime )
			CheckStuck( pBot, cmd );
		
		if( pBot->m_flNextDealObstacles < gpGlobals->curtime )
			DealWithObstacles( pBot, tr_front.m_pEnt, cmd );	

		BotNavigation( pBot, cmd );

		CheckNavMeshAttrib(pBot, &tr_front, cmd);		
	}

	// debug waypoint related position
	/*for( int i=0; i<pBot->m_Waypoints.Count(); i++ )
	{
		NDebugOverlay::Cross3DOriented( pBot->m_Waypoints[i].Center, QAngle(0,0,0), 5*i+1, 200, 0, 0, false, -1 );
	}*/

	RunPlayerMove( pBot, cmd, gpGlobals->frametime );		
}


