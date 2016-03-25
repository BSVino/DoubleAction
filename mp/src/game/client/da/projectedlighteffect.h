//========= Copyright 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#pragma once

struct dlight_t;


class CProjectedLightEffect
{
public:

	CProjectedLightEffect(int nEntIndex = 0, const char *pszTextureName = NULL, float flFov = 0.0f, float flFarZ = 0.0f, float flLinearAtten = 0.0f );
	~CProjectedLightEffect();

	void UpdateLight( int nEntIdx, const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float flFov, 
		float flFarZ, float flLinearAtten, bool castsShadows, const char* pTextureName );
	void UpdateLight(	int nEntIdx, const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float flFov,
		bool castsShadows, ITexture *pFlashlightTexture, const Vector &vecBrightness, bool bTracePlayers = true );

	void TurnOn();
	void TurnOff();
	void SetMuzzleFlashEnabled( bool bEnabled, float flBrightness );
	bool IsOn( void ) { return m_bIsOn;	}

	ClientShadowHandle_t GetFlashlightHandle( void ) { return m_FlashlightHandle; }
	void SetFlashlightHandle( ClientShadowHandle_t Handle ) { m_FlashlightHandle = Handle;	}

	const char *GetFlashlightTextureName( void ) const
	{
		return m_textureName;
	}

	int GetEntIndex( void ) const
	{
		return m_nEntIndex;
	}

protected:

	bool UpdateDefaultFlashlightState(	FlashlightState_t& state, const Vector &vecPos, const Vector &vecDir, const Vector &vecRight,
		const Vector &vecUp, bool castsShadows, bool bTracePlayers = true );
	bool ComputeLightPosAndOrientation( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp,
		Vector& vecFinalPos, Quaternion& quatOrientation, bool bTracePlayers );
	void LightOff();

	void UpdateFlashlightTexture( const char* pTextureName );

	bool m_bIsOn;
	int m_nEntIndex;
	ClientShadowHandle_t m_FlashlightHandle;

	bool m_bMuzzleFlashEnabled;
	float m_flMuzzleFlashBrightness;

	float m_flFov;
	float m_flFarZ;
	float m_flLinearAtten;
	bool  m_bCastsShadows;

	float m_flCurrentPullBackDist;

	// Texture for flashlight
	CTextureReference m_FlashlightTexture;

	// Texture for muzzle flash
	CTextureReference m_MuzzleFlashTexture;

	char m_textureName[64];
};

class CProjectedLightEffectManager
{
private:
	CProjectedLightEffect *m_pFlashlightEffect;
	const char *m_pFlashlightTextureName;
	int m_nFlashlightEntIndex;
	float m_flFov;
	float m_flFarZ;
	float m_flLinearAtten;
	int m_nMuzzleFlashFrameCountdown;
	CountdownTimer m_muzzleFlashTimer;
	float m_flMuzzleFlashStart;
	float m_flInitialMuzzleFlashBrightness;
	float m_flMuzzleFlashBrightness;
	bool m_bFlashlightOn;
	int m_nFXComputeFrame;
	bool m_bFlashlightOverride;

public:
	CProjectedLightEffectManager() : m_pFlashlightEffect( NULL ), m_pFlashlightTextureName( NULL ), m_nFlashlightEntIndex( -1 ), m_flFov( 0.0f ),
		m_flFarZ( 0.0f ), m_flLinearAtten( 0.0f ), m_nMuzzleFlashFrameCountdown( 0 ), m_flInitialMuzzleFlashBrightness( 1.0f ), m_flMuzzleFlashBrightness( 1.0f ),
		m_bFlashlightOn( false ), m_nFXComputeFrame( -1 ), m_bFlashlightOverride( false ) {}

	void TurnOnFlashlight( int nEntIndex = 0, const char *pszTextureName = NULL, float flFov = 0.0f, float flFarZ = 0.0f, float flLinearAtten = 0.0f )
	{
		m_pFlashlightTextureName = pszTextureName;
		m_nFlashlightEntIndex = nEntIndex;
		m_flFov = flFov;
		m_flFarZ = flFarZ;
		m_flLinearAtten = flLinearAtten;
		m_bFlashlightOn = true;

		if ( m_bFlashlightOverride )
		{
			// somebody is overriding the flashlight. We're keeping around the params to restore it later.
			return;
		}

		if ( !m_pFlashlightEffect )
		{
			if( pszTextureName )
			{
				m_pFlashlightEffect = new CProjectedLightEffect( m_nFlashlightEntIndex, pszTextureName, flFov, flFarZ, flLinearAtten );
			}
			else
			{
				m_pFlashlightEffect = new CProjectedLightEffect( m_nFlashlightEntIndex );
			}

			if( !m_pFlashlightEffect )
			{
				return;
			}
		}

		m_pFlashlightEffect->TurnOn();
	}

	void TurnOffFlashlight( bool bForce = false )
	{
		m_pFlashlightTextureName = NULL;
		m_bFlashlightOn = false;

		if ( bForce )
		{
			m_bFlashlightOverride = false;
			m_nMuzzleFlashFrameCountdown = 0;
			m_muzzleFlashTimer.Invalidate();
			m_flMuzzleFlashStart = 0;
			delete m_pFlashlightEffect;
			m_pFlashlightEffect = NULL;
			return;
		}

		if ( m_bFlashlightOverride )
		{
			// don't mess with it while it's overridden
			return;
		}

		if( m_nMuzzleFlashFrameCountdown == 0 && m_muzzleFlashTimer.IsElapsed() )
		{
			delete m_pFlashlightEffect;
			m_pFlashlightEffect = NULL;
		}
	}

	bool IsFlashlightOn() const { return m_bFlashlightOn; }

	void UpdateFlashlight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float flFov, bool castsShadows, float flFarZ, float flLinearAtten, const char* pTextureName = NULL );

	void SetEntityIndex( int index )
	{
		m_nFlashlightEntIndex = index;
	}

	void TriggerMuzzleFlash();

	const char *GetFlashlightTextureName( void ) const
	{
		return m_pFlashlightTextureName;
	}

	int GetFlashlightEntIndex( void ) const
	{
		return m_nFlashlightEntIndex;
	}

	void EnableFlashlightOverride( bool bEnable )
	{
		m_bFlashlightOverride = bEnable;

		if ( !m_bFlashlightOverride )
		{
			// make sure flashlight is in its original state
			if ( m_bFlashlightOn && m_pFlashlightEffect == NULL )
			{
				TurnOnFlashlight( m_nFlashlightEntIndex, m_pFlashlightTextureName, m_flFov, m_flFarZ, m_flLinearAtten );
			}
			else if ( !m_bFlashlightOn && m_pFlashlightEffect )
			{
				delete m_pFlashlightEffect;
				m_pFlashlightEffect = NULL;
			}
		}
	}

	void UpdateFlashlightOverride(	bool bFlashlightOn, const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp,
		float flFov, bool castsShadows, ITexture *pFlashlightTexture, const Vector &vecBrightness )
	{
		Assert( m_bFlashlightOverride );
		if ( !m_bFlashlightOverride )
		{
			return;
		}

		if ( bFlashlightOn && !m_pFlashlightEffect )
		{
			m_pFlashlightEffect = new CProjectedLightEffect( m_nFlashlightEntIndex );			
		}
		else if ( !bFlashlightOn && m_pFlashlightEffect )
		{
			delete m_pFlashlightEffect;
			m_pFlashlightEffect = NULL;
		}

		if( m_pFlashlightEffect )
		{
			m_pFlashlightEffect->UpdateLight( m_nFlashlightEntIndex, vecPos, vecDir, vecRight, vecUp, flFov, castsShadows, pFlashlightTexture, vecBrightness, false );
		}
	}
};

CProjectedLightEffectManager & ProjectedLightEffectManager( int32 nPlayerIndex = -1);

// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
class CTraceFilterSkipPlayerAndViewModelProjected : public CTraceFilter
{
public:
	CTraceFilterSkipPlayerAndViewModelProjected( C_BaseEntity *pPlayer, bool bTracePlayers )
	{
		m_pPlayer = pPlayer;
		m_bSkipPlayers = !bTracePlayers;
	}

	virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask );

private:
	C_BaseEntity *m_pPlayer;
	bool m_bSkipPlayers;
};
