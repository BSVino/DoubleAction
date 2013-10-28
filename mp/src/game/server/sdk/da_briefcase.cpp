//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "da_briefcase.h"
#include "sdk_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CBriefcase )
END_DATADESC()

LINK_ENTITY_TO_CLASS( da_briefcase, CBriefcase );

IMPLEMENT_SERVERCLASS_ST(CBriefcase, DT_Briefcase)
END_SEND_TABLE()

PRECACHE_REGISTER(da_briefcase);

CBriefcase::CBriefcase()
{
}

void CBriefcase::Precache( void )
{
	CBaseEntity::PrecacheModel( "particle/briefcase.vmt" );
}

void CBriefcase::Spawn( void )
{
	BaseClass::Spawn( );
}

int CBriefcase::UpdateTransmitState()
{
	return SetTransmitState( FL_EDICT_ALWAYS );
}

bool CBriefcase::MyTouch( CBasePlayer *pPlayer )
{
	if (!pPlayer->IsAlive())
		return false;

	UTIL_Remove(this);

	return true;
}
