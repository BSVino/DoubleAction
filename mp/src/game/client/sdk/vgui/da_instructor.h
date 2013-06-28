#pragma once

#include <vgui_controls/Panel.h>

#include "hudelement.h"

typedef bool (*pfnConditionsMet)( class C_SDKPlayer *pPlayer, class CLesson *pLesson );
pfnConditionsMet Game_GetInstructorConditions(const CUtlString& sConditions);

class CLessonOutput
{
public:
	CLessonOutput()
	{
	};

	CUtlString    m_sOutput;
	CUtlString    m_sTarget;
	CUtlString    m_sInput;
	CUtlString    m_sArgs;
};

class CLesson
{
public:
	typedef enum
	{
		LEARN_DISPLAYING,
		LEARN_PERFORMING,
	} learn_t;

	typedef enum
	{
		LESSON_BUTTON,      // Tell the player about a button. Lower center display. Icon to display is interpreted as a button.
		LESSON_INFO,        // Tell the player some information. Lower center display. Icon is a regular texture.
		LESSON_ENVIRONMENT, // Point to the player something in his environment. Custom painting must be done.
	} lesson_t;

public:
	CLesson(class CInstructor* pInstructor, const CUtlString& sLesson);

public:
	class CInstructor*              m_pInstructor;
	CUtlString                      m_sLessonName;
	CUtlString                      m_sNextLesson;
	CUtlString                      m_sText;
	int                             m_iWidth;
	bool                            m_bAutoNext;
	bool                            m_bKillOnFinish;
	float                           m_flSlideAmount;
	bool                            m_bSlideX;

	int                             m_iPriority;
	CUtlVector<CUtlString>          m_asPrerequisites;

	lesson_t                        m_iLessonType;
	learn_t                         m_iLearningMethod;
	int                             m_iTimesToLearn;

	pfnConditionsMet                m_pfnConditions;
};

class CHudLessonPanel : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudLessonPanel, vgui::Panel );

public:
	CHudLessonPanel( const char *pElementName );

	void Init();
	void Reset();
	void MsgFunc_HintText( bf_read &msg );
	void FireGameEvent( IGameEvent * event);

	bool SetHintText( wchar_t *text );
	void LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString );

	virtual void PerformLayout();

	void SetLesson( CLesson* pLesson );

protected:
	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	virtual void OnThink();

protected:
	vgui::HFont  m_hFont;
	Color        m_bgColor;
	vgui::Label* m_pLabel;
	CUtlVector<vgui::Label *> m_Labels;

	CPanelAnimationVarAliasType( int, m_iTextX, "text_xpos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iTextY, "text_ypos", "8", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterX, "center_x", "0", "proportional_int" );
	CPanelAnimationVarAliasType( int, m_iCenterY, "center_y", "0", "proportional_int" );

	bool        m_bLastLabelUpdateHack;
	CPanelAnimationVar( float, m_flLabelSizePercentage, "HintSize", "0" );

	CLesson*    m_pLesson;
};

class CInstructor
{
	friend class CLessonPanel;

public:
	CInstructor();
	~CInstructor();

public:
	void            Clear();
	void            Initialize();
	bool            IsInitialized() { return !!m_apLessons.Count(); }

	void            ReadLesson(class KeyValues* pData);

	const CUtlMap<CUtlString, CLesson*>& GetLessons() const { return m_apLessons; }
	CLesson*        GetLesson(const CUtlString& sLesson);

	void            SetActive(bool bActive);
	bool            GetActive() { return m_bActive; };

	void            DisplayFirstLesson(const CUtlString& sLesson);
	void            NextLesson();

	void            DisplayLesson(const CUtlString& sLesson);
	void            HideLesson();
	void            ShowLesson();
	void            FinishedLesson(const CUtlString& sLesson, bool bForceNext = false);

	CLesson*        GetCurrentLesson() { return m_apLessons[m_apLessons.Find(m_sCurrentLesson.Get())]; };

	static pfnConditionsMet GetBaseConditions(const CUtlString& sCondition);

protected:
	bool                        m_bActive;
	CUtlMap<CUtlString, CLesson*> m_apLessons;
	CUtlString                  m_sLastLesson;
	CUtlString                  m_sCurrentLesson;
};
