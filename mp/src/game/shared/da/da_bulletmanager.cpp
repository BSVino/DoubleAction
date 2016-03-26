#include "cbase.h"

#include "da_bulletmanager.h"

#include "engine/ivdebugoverlay.h"
#include "effect_dispatch_data.h"
#include "ammodef.h"

#ifdef CLIENT_DLL
#include "view.h"
#include "model_types.h"
#endif

#include "da_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CBulletManager g_BulletManager( "CBulletManager" );

CBulletManager& BulletManager()
{
	return g_BulletManager;
}

CBulletManager::CBulletManager( char const* name )
	: CAutoGameSystemPerFrame(name)
{
}

void CBulletManager::LevelInitPostEntity()
{
	m_aBullets.SetSize(200);

	for (int i = 0; i < m_aBullets.Count(); i++)
		m_aBullets[i].Deactivate();
}

CBulletManager::CBullet CBulletManager::MakeBullet(CDAPlayer* pShooter, const Vector& vecSrc, const Vector& vecDirection, DAWeaponID eWeapon, CWeaponDABase* pWeapon, int iDamage, int iBulletType, bool bDoEffects)
{
	CBullet oBullet;

	oBullet.m_bActive = true;

	oBullet.m_hShooter = pShooter;
	oBullet.m_hWeapon = pWeapon;
	oBullet.m_flShotTime = gpGlobals->curtime;
	oBullet.m_vecOrigin = vecSrc;
	oBullet.m_vecDirection = vecDirection;
	oBullet.m_eWeaponID = eWeapon;
	oBullet.m_bDoEffects = bDoEffects;
	oBullet.m_iBulletType = iBulletType;
	oBullet.m_iBulletDamage = iDamage;
	oBullet.m_ahObjectsHit.RemoveAll();

	return oBullet;
}

void CBulletManager::AddBullet(const CBullet& oBullet)
{
#ifdef CLIENT_DLL
	// If we're on the client then our only job is to do effects.
	// If this is false on the client it means the client prediction is running,
	// so don't bother. It's always false on the server but we
	// should still do the bullet since we need to do damage etc.
	if (!oBullet.m_bDoEffects)
		return;
#endif

	// Find an empty spot
	int iEmpty = -1;
	for (int i = 0; i < m_aBullets.Count(); i++)
	{
		if (!m_aBullets[i].m_bActive)
		{
			iEmpty = i;
			break;
		}
	}

	Assert(iEmpty >= 0);
	if (iEmpty < 0)
		return;

	m_aBullets[iEmpty] = oBullet;
	m_aBullets[iEmpty].Activate();
}

void CBulletManager::Update(float frametime)
{
	BulletsThink(frametime);
}

void CBulletManager::FrameUpdatePreEntityThink()
{
	BulletsThink(gpGlobals->frametime);
}

ConVar da_bullet_speed("da_bullet_speed", "4000", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How fast do bullets go during slow motion?" );
ConVar da_bullet_speed_active("da_bullet_speed_active", "6000", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How fast do bullets go during slow motion?" );
ConVar da_bullet_debug("da_bullet_debug", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Shows client (red) and server (blue) bullet path" );

void CBulletManager::BulletsThink(float flFrameTime)
{
	for (int i = 0; i < m_aBullets.Count(); i++)
	{
		CBullet& oBullet = m_aBullets[i];
		if (!oBullet.m_bActive)
			continue;

		if (!oBullet.m_hShooter)
			oBullet.Deactivate();

		float flSpeed = da_bullet_speed.GetFloat();
		float flLerpTime = flSpeed / 400 * flFrameTime;

		if (oBullet.m_hShooter)
		{
			if (oBullet.m_hShooter->m_Shared.m_bSuperSkill || oBullet.m_hShooter->GetSlowMoType() == SLOWMO_ACTIVATED || oBullet.m_hShooter->GetSlowMoType() == SLOWMO_STYLESKILL)
				flSpeed = da_bullet_speed_active.GetFloat();

				flLerpTime *= oBullet.m_hShooter->GetSlowMoMultiplier();
		}

		oBullet.m_flCurrAlpha = Approach(oBullet.m_flGoalAlpha, oBullet.m_flCurrAlpha, flLerpTime);

		if (oBullet.m_bActive && oBullet.m_flGoalAlpha == 0 && oBullet.m_flCurrAlpha == 0)
		{
			oBullet.m_bActive = false;
#ifdef CLIENT_DLL
			ClientLeafSystem()->RemoveRenderable( oBullet.m_hRenderHandle );
#endif
			continue;
		}

		if (gpGlobals->curtime > oBullet.m_flShotTime + 10)
		{
			oBullet.Deactivate();
			continue;
		}

		if (!oBullet.m_hShooter)
			continue;

		float dt;
		if (oBullet.m_hShooter->GetSlowMoMultiplier() == 1)
			dt = -1;
		else
			dt = flSpeed * oBullet.m_hShooter->GetSlowMoMultiplier() * flFrameTime;

		SimulateBullet(oBullet, dt);
	}
}

ConVar sv_showimpacts("sv_showimpacts", "0", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "Shows client (red) and server (blue) bullet impact point" );
ConVar da_bullet_penetrations("da_bullet_penetrations", "5", FCVAR_REPLICATED|FCVAR_CHEAT|FCVAR_DEVELOPMENTONLY, "How many times will a bullet penetrate a wall?" );

void DispatchEffect( const char *pName, const CEffectData &data );

void CBulletManager::SimulateBullet(CBullet& oBullet, float dt)
{
	Vector vecOriginal = oBullet.m_vecOrigin;

	Assert(oBullet.m_hShooter.Get());
	if (!oBullet.m_hShooter)
		return;

	bool bHasTraveledBefore = false;
	if (oBullet.m_flDistanceTraveled > 0)
		bHasTraveledBefore = true;

	// initialize these before the penetration loop, we'll need them to make our tracer after
	Vector vecTracerSrc = oBullet.m_vecOrigin;
	trace_t tr; // main enter bullet trace

	float flRange = dt;

	if (flRange < 0)
		flRange = 8000;

	bool bFullPenetrationDistance = false;

	Vector vecEnd = oBullet.m_vecOrigin + oBullet.m_vecDirection * flRange;

	int i;
	for (i = oBullet.m_iPenetrations; i < da_bullet_penetrations.GetInt(); i++)
	{
		CTraceFilterSimpleList tf(COLLISION_GROUP_NONE);
		tf.AddEntityToIgnore(oBullet.m_hShooter);

		for (int j = 0; j < oBullet.m_ahObjectsHit.Count(); j++)
			tf.AddEntityToIgnore(oBullet.m_ahObjectsHit[j]);

		UTIL_TraceLine( oBullet.m_vecOrigin, vecEnd, MASK_SOLID|CONTENTS_DEBRIS|CONTENTS_HITBOX, &tf, &tr );

		if (da_bullet_debug.GetBool())
		{
#ifdef CLIENT_DLL
			DebugDrawLine(oBullet.m_vecOrigin + Vector(0, 0, 1), tr.endpos + Vector(0, 0, 1), 0, 255, 255, true, dt<0?10:0.1);
#else
			DebugDrawLine(oBullet.m_vecOrigin + Vector(0, 0, 1), tr.endpos + Vector(0, 0, 1), 255, 255, 0, true, dt<0?10:0.1);
#endif
		}

		Vector vecTraceEnd = tr.endpos;

		bool bBSPModel = tr.DidHitWorld();

		if (tr.allsolid)
		{
			oBullet.m_flDistanceTraveled += (oBullet.m_vecOrigin - vecEnd).Length();
			oBullet.m_vecOrigin = vecEnd;
			break; // We're inside something. Do nothing.
		}

		if ( sv_showimpacts.GetBool() && tr.fraction < 1.0f )
		{
#ifdef CLIENT_DLL
			// draw red client impact markers
			debugoverlay->AddBoxOverlay( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), QAngle( 0, 0, 0), 255,0,0,127, sv_showimpacts.GetFloat() );

			if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
			{
				C_BasePlayer *player = ToBasePlayer( tr.m_pEnt );
				player->DrawClientHitboxes( sv_showimpacts.GetFloat(), true );
			}
#else
			// draw blue server impact markers
			NDebugOverlay::Box( tr.endpos, Vector(-2,-2,-2), Vector(2,2,2), 0,0,255,127, sv_showimpacts.GetFloat() );

			if ( tr.m_pEnt && tr.m_pEnt->IsPlayer() )
			{
				CBasePlayer *player = ToBasePlayer( tr.m_pEnt );
				player->DrawServerHitboxes( sv_showimpacts.GetFloat(), true );
			}
#endif
		}

		Assert(oBullet.m_iBulletType > 0);
		int iDamageType = DMG_BULLET | DMG_NEVERGIB | GetAmmoDef()->DamageType(oBullet.m_iBulletType);

		if (i == 0)
			iDamageType |= DMG_DIRECT;

		if (tr.startsolid)
		{
			trace_t tr2;

			UTIL_TraceLine( tr.endpos - oBullet.m_vecDirection, oBullet.m_vecOrigin, CONTENTS_SOLID|CONTENTS_MOVEABLE, NULL, COLLISION_GROUP_NONE, &tr2 );

			// let's have a bullet exit effect if we penetrated a solid surface
			if (oBullet.m_bDoEffects && tr2.m_pEnt && tr2.m_pEnt->IsBSPModel())
				UTIL_ImpactTrace( &tr2, iDamageType );

			// ignore the entity we just hit for the next trace to avoid weird impact behaviors
			oBullet.m_ahObjectsHit.AddToTail(tr2.m_pEnt);
		}

		if ( tr.fraction == 1.0f )
		{
			oBullet.m_flDistanceTraveled += (oBullet.m_vecOrigin - vecEnd).Length();
			oBullet.m_vecOrigin = vecEnd;
			break; // we didn't hit anything, stop tracing shoot
		}

		weapontype_t eWeaponType = WT_NONE;

		CSDKWeaponInfo *pWeaponInfo = CSDKWeaponInfo::GetWeaponInfo(oBullet.m_eWeaponID);
		Assert(pWeaponInfo);
		if (pWeaponInfo)
			eWeaponType = pWeaponInfo->m_eWeaponType;

		float flDamageMultiplier = 1;
		float flMaxRange = 3000;

		// Power formula works like so:
		// pow( x, distance/y )
		// The damage will be at 1 when the distance is 0 units, and at
		// x% when the distance is y units, with a gradual decay approaching zero
		switch (eWeaponType)
		{
		case WT_RIFLE:
			flDamageMultiplier = 0.75f;
			flMaxRange = 3000;
			break;

		case WT_SHOTGUN:
			flDamageMultiplier = 0.40f;
			flMaxRange = 500;
			break;

		case WT_SMG:
			flDamageMultiplier = 0.50f;
			flMaxRange = 1000;
			break;

		case WT_PISTOL:
		default:
			flDamageMultiplier = 0.55f;
			flMaxRange = 1500;
			break;
		}

		flMaxRange *= oBullet.m_hShooter->m_Shared.ModifySkillValue(1, 0.5f, SKILL_MARKSMAN);

		//calculate the damage based on the distance the bullet travelled.
		oBullet.m_flDistanceTraveled += tr.fraction * flRange;

		float flCurrentDistance = oBullet.m_flDistanceTraveled;

		// First 500 units, no decrease in damage.
		if (eWeaponType == WT_SHOTGUN)
			flCurrentDistance -= 350;
		else
			flCurrentDistance -= 500;

		if (flCurrentDistance < 0)
			flCurrentDistance = 0;

		if (flCurrentDistance > flMaxRange)
			flCurrentDistance = flMaxRange;

		float flDistanceMultiplier = pow(flDamageMultiplier, (flCurrentDistance / flMaxRange));

		if( oBullet.m_bDoEffects )
		{
			// See if the bullet ended up underwater + started out of the water
			if ( enginetrace->GetPointContents( tr.endpos ) & (CONTENTS_WATER|CONTENTS_SLIME) )
			{
				CBaseEntity* pIgnore;
				if (oBullet.m_ahObjectsHit.Count())
					pIgnore = oBullet.m_ahObjectsHit.Tail();
				else
					pIgnore = oBullet.m_hShooter;

				trace_t waterTrace;
				UTIL_TraceLine( oBullet.m_vecOrigin, tr.endpos, (MASK_SHOT|CONTENTS_WATER|CONTENTS_SLIME), pIgnore, COLLISION_GROUP_NONE, &waterTrace );

				if( waterTrace.allsolid != 1 )
				{
					CEffectData	data;
					data.m_vOrigin = waterTrace.endpos;
					data.m_vNormal = waterTrace.plane.normal;
					data.m_flScale = random->RandomFloat( 8, 12 );

					if ( waterTrace.contents & CONTENTS_SLIME )
						data.m_fFlags |= FX_WATER_IN_SLIME;

					DispatchEffect( "gunshotsplash", data );
				}
			}
			else
			{
				//Do Regular hit effects

				// Don't decal nodraw surfaces
				if ( !( tr.surface.flags & (SURF_SKY|SURF_NODRAW|SURF_HINT|SURF_SKIP) ) )
				{
					CBaseEntity *pEntity = tr.m_pEnt;
					//Tony; only while using teams do we check for friendly fire.
					if ( DAGameRules()->IsTeamplay() && pEntity && pEntity->IsPlayer() && (pEntity->GetBaseAnimating() && !pEntity->GetBaseAnimating()->IsRagdoll()) )
					{
						if ( pEntity->GetTeamNumber() != oBullet.m_hShooter->GetTeamNumber() )
							UTIL_ImpactTrace( &tr, iDamageType );
					}
					//Tony; non player, just go nuts,
					else
						UTIL_ImpactTrace( &tr, iDamageType );
				}
			}
		} // bDoEffects

		// add damage to entity that we hit

#ifdef GAME_DLL
		float flBulletDamage = oBullet.m_iBulletDamage * flDistanceMultiplier / (i+1);	// Each iteration the bullet drops in strength
		if (oBullet.m_hShooter->IsStyleSkillActive(SKILL_MARKSMAN))
			flBulletDamage = oBullet.m_iBulletDamage * flDistanceMultiplier / (i/2+1);	// Each iteration the bullet drops in strength but not nearly as much.

		ClearMultiDamage();

		CTakeDamageInfo info( oBullet.m_hShooter, oBullet.m_hShooter, oBullet.m_hWeapon, flBulletDamage, iDamageType );
		CalculateBulletDamageForce( &info, oBullet.m_iBulletType, oBullet.m_vecDirection, tr.endpos );
		tr.m_pEnt->DispatchTraceAttack( info, oBullet.m_vecDirection, &tr );

		oBullet.m_hShooter->TraceAttackToTriggers( info, tr.startpos, tr.endpos, oBullet.m_vecDirection );

		ApplyMultiDamage();
#else
		flDistanceMultiplier = flDistanceMultiplier; // Silence warning.
#endif

		if (tr.m_pEnt && !FStrEq(tr.m_pEnt->GetClassname(), "worldspawn"))
			oBullet.m_ahObjectsHit.AddToTail(tr.m_pEnt);

		float flPenetrationDistance;
		switch (eWeaponType)
		{
		case WT_RIFLE:
			flPenetrationDistance = 25;
			break;

		case WT_SHOTGUN:
			flPenetrationDistance = 5;
			break;

		case WT_SMG:
			flPenetrationDistance = 15;
			break;

		case WT_PISTOL:
		default:
			flPenetrationDistance = 15;
			break;
		}

		flPenetrationDistance = oBullet.m_hShooter->m_Shared.ModifySkillValue(flPenetrationDistance, 1, SKILL_MARKSMAN);

		Vector vecBackwards = tr.endpos + oBullet.m_vecDirection * flPenetrationDistance;
		if (tr.m_pEnt->IsBSPModel())
			UTIL_TraceLine( vecBackwards, tr.endpos, CONTENTS_SOLID|CONTENTS_MOVEABLE, NULL, COLLISION_GROUP_NONE, &tr );
		else
			UTIL_TraceLine( vecBackwards, tr.endpos, CONTENTS_HITBOX, NULL, COLLISION_GROUP_NONE, &tr );

		if (tr.startsolid)
		{
			bFullPenetrationDistance = true;
			break;
		}

		// Set up the next trace. One unit in the direction of fire so that we firmly embed
		// ourselves in whatever solid was hit, to make sure we don't hit it again on next trace.
		if (dt < 0 && bBSPModel)
		{
			UTIL_TraceLine( vecTraceEnd + oBullet.m_vecDirection, vecTraceEnd + oBullet.m_vecDirection * flPenetrationDistance, CONTENTS_SOLID|CONTENTS_MOVEABLE, NULL, COLLISION_GROUP_NONE, &tr );

			if (tr.startsolid)
				oBullet.m_vecOrigin = tr.startpos + oBullet.m_vecDirection;
			else
				oBullet.m_vecOrigin = vecTraceEnd + oBullet.m_vecDirection;
		}
		else
			oBullet.m_vecOrigin = vecTraceEnd + oBullet.m_vecDirection;
	}

	oBullet.m_iPenetrations = i;

	// the bullet's done penetrating, let's spawn our particle system
	if (oBullet.m_bDoEffects && dt < 0)
		oBullet.m_hShooter->MakeTracer( oBullet.m_vecOrigin, tr, TRACER_TYPE_DEFAULT, !bHasTraveledBefore );

#ifdef CLIENT_DLL
	if (oBullet.m_hRenderHandle != INVALID_CLIENT_RENDER_HANDLE)
		ClientLeafSystem()->RenderableChanged( oBullet.m_hRenderHandle );
#endif

	if (bFullPenetrationDistance || oBullet.m_iPenetrations >= da_bullet_penetrations.GetInt())
		oBullet.Deactivate();

	if (dt < 0)
		oBullet.Deactivate();

	if (!bHasTraveledBefore && oBullet.m_flCurrAlpha == 0 && oBullet.m_flGoalAlpha == 0)
		oBullet.m_bActive = false;

	if (da_bullet_debug.GetBool())
	{
#ifdef CLIENT_DLL
		DebugDrawLine(vecOriginal, oBullet.m_vecOrigin, 0, 0, 255, true, dt<0?10:0.1);
#else
		DebugDrawLine(vecOriginal, oBullet.m_vecOrigin, 255, 0, 0, true, dt<0?10:0.1);
#endif
	}
}

#ifdef CLIENT_DLL
const Vector& CBulletManager::CBullet::GetRenderOrigin()
{
	return m_vecOrigin;
}

const QAngle& CBulletManager::CBullet::GetRenderAngles()
{
	return vec3_angle;
}

const matrix3x4_t& CBulletManager::CBullet::RenderableToWorldTransform()
{
	static matrix3x4_t mat;
	SetIdentityMatrix( mat );
	PositionMatrix( GetRenderOrigin(), mat );
	return mat;
}

bool CBulletManager::CBullet::ShouldDraw( void )
{
	return true;
}

bool CBulletManager::CBullet::IsTransparent( void )
{
	return true;
}

void CBulletManager::CBullet::GetRenderBounds( Vector& mins, Vector& maxs )
{
	mins = Vector(-10, -10, -10);
	maxs = Vector(10, 10, 10);
}

void DrawCross(CMeshBuilder& meshBuilder, const Vector& vecOrigin, const Vector& vecRight, const Vector& vecDirection, const float& flAlpha)
{
	float flWidth = 2.5f;
	float flLength = 250;

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,0, 0 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * flWidth) - (vecDirection * flLength)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,1, 0 );
	meshBuilder.Position3fv( (vecOrigin - (vecRight * flWidth) - (vecDirection * flLength)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,1, 1 );
	meshBuilder.Position3fv( (vecOrigin - (vecRight * flWidth)).Base() );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color4f( 1, 1, 1, flAlpha );
	meshBuilder.TexCoord2f( 0,0, 1 );
	meshBuilder.Position3fv( (vecOrigin + (vecRight * flWidth)).Base() );
	meshBuilder.AdvanceVertex();
}

CMaterialReference g_hBulletStreak;
int CBulletManager::CBullet::DrawModel( int flags )
{
	if (m_flCurrAlpha < 0)
		return 0;

	if (flags & STUDIO_SHADOWDEPTHTEXTURE)
		return 0;

#ifdef __linux__
	return 0;
#endif

	if (!g_hBulletStreak.IsValid())
		g_hBulletStreak.Init( "effects/tracer1.vmt", TEXTURE_GROUP_OTHER );

	float flAlpha = 150.5f/255.0f * m_flCurrAlpha;

	Vector vecRight = Vector(0, 0, 1).Cross(m_vecDirection).Normalized();
	Vector vecCross1 = (Vector(0, 0, 1) + vecRight).Normalized();
	Vector vecCross2 = (Vector(0, 0, 1) - vecRight).Normalized();

	CMeshBuilder meshBuilder;

	CMatRenderContextPtr pRenderContext( materials );
	pRenderContext->Bind( g_hBulletStreak );
	IMesh* pMesh = pRenderContext->GetDynamicMesh();

	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	DrawCross(meshBuilder, GetRenderOrigin(), vecCross1, m_vecDirection, flAlpha);
	DrawCross(meshBuilder, GetRenderOrigin(), vecCross2, m_vecDirection, flAlpha);

	meshBuilder.End(false, true);

	return 1;
}
#endif

void CBulletManager::CBullet::Activate()
{
	m_bActive = true;

	m_flGoalAlpha = 1;

#ifdef CLIENT_DLL
	if (m_hShooter == C_DAPlayer::GetLocalOrSpectatedPlayer())
		m_flCurrAlpha = -0.3f; // Don't appear until it's a little ways from the player.
	else
		m_flCurrAlpha = 0.0f;
#else
	m_flCurrAlpha = 0;
#endif

#ifdef CLIENT_DLL
	ClientLeafSystem()->AddRenderable( this, RENDER_GROUP_TRANSLUCENT_ENTITY );
#endif
}

void CBulletManager::CBullet::Deactivate()
{
	m_bDoEffects = false;
	m_iBulletDamage = 0;
	m_flGoalAlpha = 0;
}
