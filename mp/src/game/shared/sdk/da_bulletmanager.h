#pragma once

#ifdef CLIENT_DLL
#include "c_sdk_player.h"
#else
#include "sdk_player.h"
#endif

class CBulletManager : public CAutoGameSystemPerFrame
{
public:
	CBulletManager( char const *name );

public:
	class CBullet
	{
	public:
		CBullet()
		{
			m_bAvailable = true;
			m_iPenetrations = 0;
			m_eWeaponID = WEAPON_NONE;
			m_flDistanceTraveled = 0;
			m_bDoEffects = false;
			m_iBulletType = -1;
		}

	public:
		bool        m_bAvailable;
		CHandle<CSDKPlayer> m_hShooter;
		float       m_flShotTime;
		Vector      m_vecOrigin;
		Vector      m_vecDirection;
		int         m_iPenetrations;
		SDKWeaponID m_eWeaponID;
		float       m_flDistanceTraveled;
		bool        m_bDoEffects;
		int         m_iBulletType;
		int         m_iBulletDamage;
		CCopyableUtlVector<CHandle<CBaseEntity>> m_ahObjectsHit;
	};

public:
	virtual void LevelInitPostEntity();

	CBullet MakeBullet(CSDKPlayer* pShooter, const Vector& vecSrc, const Vector& vecDirection, SDKWeaponID eWeapon, int iDamage, int iBulletType, bool bDoEffects);
	void AddBullet(const CBullet& oBullet);

	virtual void Update( float frametime );
	virtual void FrameUpdatePreEntityThink();

	void BulletsThink(float flFrameTime);
	void SimulateBullet(CBullet& oBullet, float dt);

private:
	CUtlVector<CBullet> m_aBullets;
};

CBulletManager& BulletManager();
