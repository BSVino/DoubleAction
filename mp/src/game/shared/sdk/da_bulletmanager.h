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
#ifdef CLIENT_DLL
		: public CDefaultClientRenderable
#endif
	{
	public:
		CBullet()
		{
			m_bActive = false;
			m_iPenetrations = 0;
			m_eWeaponID = WEAPON_NONE;
			m_flDistanceTraveled = 0;
			m_bDoEffects = false;
			m_iBulletType = -1;
			m_flGoalAlpha = 0;
			m_flCurrAlpha = 0;
		}

#ifdef CLIENT_DLL
	// IClientRenderable
	public:
		virtual const Vector&      GetRenderOrigin( void );
		virtual const QAngle&      GetRenderAngles( void );
		virtual const matrix3x4_t& RenderableToWorldTransform();
		virtual bool               ShouldDraw( void );
		virtual bool               IsTransparent( void );
		virtual void               GetRenderBounds( Vector& mins, Vector& maxs );
		virtual int                DrawModel( int flags );
#endif

	public:
		void        Activate();
		void        Deactivate();

	public:
		bool        m_bActive;
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
		float       m_flGoalAlpha;
		float       m_flCurrAlpha;
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
