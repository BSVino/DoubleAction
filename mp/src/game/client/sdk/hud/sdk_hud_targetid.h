#pragma once

class CTarget
{
public:
	CHudTexture*    m_pTargetTexture = nullptr;
	wchar_t*        m_pwszHint = nullptr;
	float           m_flTargetAlpha = 0.0f;
	float           m_flScale;
	float           m_flMaxAlpha;
	Vector          m_vecLastKnownTarget;
	bool            m_bTargetOn = false;
	EHANDLE         m_hEntity;
	bool            m_bHideIfVisible = true;
	vgui::HFont     m_hFont;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CSDKTargetId : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CSDKTargetId, vgui::Panel);

public:
	CSDKTargetId(const char *pElementName);
	void Init(void);
	virtual void	ApplySchemeSettings(vgui::IScheme *scheme);
	virtual void	Paint(void);
	void VidInit(void);

private:
	void PaintTargets(CTarget* pTargets, int iCount);

private:
	Color			GetColorForTargetTeam(int iTeamNumber);

	CPanelAnimationVar(vgui::HFont, m_hFont, "TargetIDFont", "Default");

	int				m_iLastEntIndex;
	float			m_flLastChangeTime;

	CHudTexture*    m_pBriefcase;
	CHudTexture*    m_pCapturePoint;
	CHudTexture*    m_pBounty;

	typedef enum
	{
		TARGET_BRIEFCASE = 0,
		TARGET_CAPTURE,
		TARGET_BOUNTY,
		TARGET_WAYPOINT1,
		TARGET_WAYPOINT2,
		TARGET_WAYPOINT3,
		TARGET_LEADER,
		TARGET_FRONTRUNNER1,
		TARGET_FRONTRUNNER2,
		TARGET_TOTAL,
	} target_type_t;

	CTarget m_Targets[TARGET_TOTAL];
	CTarget m_RevealedEnemies[MAX_PLAYERS];

	CPanelAnimationVar(vgui::HFont, m_hMiniObjectiveFont, "MiniObjectiveFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hMiniObjectiveFontSmall, "MiniObjectiveFontSmall", "Default");
};

DECLARE_HUDELEMENT(CSDKTargetId);
