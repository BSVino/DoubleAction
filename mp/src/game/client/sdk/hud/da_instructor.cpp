#include "cbase.h"

#include "filesystem.h"
#include <vgui/ILocalize.h>
#include "iclientmode.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "vgui_controls/AnimationController.h"
#include "vgui_controls/Label.h"
#include "vgui/ISurface.h"
#include "text_message.h"
#include "sourcevr/isourcevirtualreality.h"

#include "da_instructor.h"
#include "c_sdk_player.h"
#include "da_buymenu.h"
#include "ammodef.h"
#include "da_hud_vote.h"

#undef min
#undef max

#include <string>
#include <vector>
#include <sstream>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar da_instructor_lessons_learned("da_instructor_lessons_learned", "", FCVAR_ARCHIVE);

CInstructor::CInstructor()
{
	m_bActive = true;
	m_bSideHintShowing = false;
	m_sLastLesson = "";
	Initialize();
}

CInstructor::~CInstructor()
{
	HideLesson();
	Clear();
}

static bool Str_LessFunc( CUtlString const &a, CUtlString const &b )
{
	return strcmp(a.Get(), b.Get()) < 0;
}

void CInstructor::Clear()
{
	for (unsigned short i = m_apLessons.FirstInorder(); m_apLessons.IsValidIndex(i); i = m_apLessons.NextInorder(i))
		delete m_apLessons[i];

	m_apLessons.Purge();
	m_apLessons.RemoveAll();

	m_apLessons.SetLessFunc(&Str_LessFunc);
}

void CInstructor::Initialize()
{
	Clear();

	KeyValues *kv = new KeyValues( "instructor" );
	if ( !kv )
		return;

	if (! kv->LoadFromFile( filesystem, "scripts/instructor.txt" ) )
		return;

	KeyValues *pKVLesson = kv;
	while ( pKVLesson )
	{
		if (FStrEq(pKVLesson->GetName(), "Lesson"))
			ReadLesson(pKVLesson);

		pKVLesson = pKVLesson->GetNextKey();
	}

	kv->deleteThis();
}

void CInstructor::ReadLesson(KeyValues* pData)
{
	const char* pszLessonName = pData->GetString("Name");
	if (!strlen(pszLessonName))
	{
		Error("Found a lesson with no name.\n");
		return;
	}

	CLesson* pLesson = new CLesson(this, pszLessonName);
	m_apLessons.Insert(pszLessonName, pLesson);

	pLesson->m_iWidth = pData->GetInt("Width", 100);
	pLesson->m_sNextLesson = pData->GetString("Next", "");
	pLesson->m_sText = pData->GetString("Text", "");
	pLesson->m_sSideHintText = pData->GetString("SideHint", "");
	pLesson->m_flSlideAmount = pData->GetFloat("SlideAmount", 0);
	pLesson->m_bSlideX = pData->GetInt("SlideX", 0);
	pLesson->m_iPriority = pData->GetInt("Priority", 0);

	CUtlString sLessonType = pData->GetString("LessonType", "button");

	if (sLessonType == "button")
		pLesson->m_iLessonType = CLesson::LESSON_BUTTON;
	else if (sLessonType == "info")
		pLesson->m_iLessonType = CLesson::LESSON_INFO;
	else if (sLessonType == "environment")
		pLesson->m_iLessonType = CLesson::LESSON_ENVIRONMENT;

	CUtlString sLearningMethod = pData->GetString("LearningMethod", "performing");

	if (sLearningMethod == "displaying")
		pLesson->m_iLearningMethod = CLesson::LEARN_DISPLAYING;
	else if (sLearningMethod == "display")
		pLesson->m_iLearningMethod = CLesson::LEARN_DISPLAYING;
	else if (sLearningMethod == "performing")
		pLesson->m_iLearningMethod = CLesson::LEARN_PERFORMING;
	else if (sLearningMethod == "perform")
		pLesson->m_iLearningMethod = CLesson::LEARN_PERFORMING;

	pLesson->m_iTimesToLearn = pData->GetInt("TimesToLearn", 1);
	pLesson->m_iMaxShows = pData->GetInt("MaxShows", -1);
	pLesson->m_pfnConditions = Game_GetInstructorConditions(pData->GetString("Conditions")); // A dirty hack, but not a scary one.
}

CLesson* CInstructor::GetLesson(const CUtlString& sLesson)
{
	unsigned short iLesson = m_apLessons.Find(sLesson);
	if (iLesson == m_apLessons.InvalidIndex())
		return NULL;

	return m_apLessons[iLesson];
}

bool WhoCaresConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	return true;
}

bool ValidPlayerConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!pPlayer)
		return false;

	if (pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
		return false;

	// MOTD
	if (gViewPortInterface->FindPanelByName(PANEL_INFO)->IsVisible())
		return false;

	if (gViewPortInterface->FindPanelByName(PANEL_INTRO)->IsVisible())
		return false;

	// Choose character screen
	if (gViewPortInterface->FindPanelByName(PANEL_FOLDER)->IsVisible())
		return false;

	return true;
}

bool PlayerAliveConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!ValidPlayerConditions(pPlayer, pLesson))
		return false;

	if (!pPlayer->IsAlive())
		return false;

	if (pPlayer->GetLastSpawnTime() >= 0 && gpGlobals->curtime < pPlayer->GetLastSpawnTime() + 2)
		return false;

	return true;
}

bool PlayerDeadConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!ValidPlayerConditions(pPlayer, pLesson))
		return false;

	if (pPlayer->IsAlive())
		return false;

	if (!pPlayer->HasPlayerDied())
		return false;

	return true;
}

bool PlayerDeadNoKillCamConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerDeadConditions(pPlayer, pLesson))
		return false;

	if (pPlayer->GetObserverMode() == OBS_MODE_FREEZECAM)
		return false;

	return true;
}

bool PlayerCanReloadConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	CWeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	if (!pWeapon)
		return false;

	if (!pWeapon->UsesClipsForAmmo1())
		return false;

	if (pWeapon->Clip1() == pWeapon->GetMaxClip1())
		return false;

	return pWeapon->HasPrimaryAmmo();
}

bool PlayerHasMultipleWeaponsConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	int iWeapons = 0;
	for (int i = 0; i < pPlayer->WeaponCount(); i++)
	{
		if (!pPlayer->GetWeapon(i))
			continue;

		CWeaponSDKBase* pWeapon = dynamic_cast<CWeaponSDKBase*>(pPlayer->GetWeapon(i));
		if (!pWeapon)
			continue;

		if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
			continue;

		iWeapons++;
	}

	return iWeapons > 1;
}

bool PlayerOutOfAmmoAndMultipleWeaponsConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerHasMultipleWeaponsConditions(pPlayer, pLesson))
		return false;

	CWeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	if (!pWeapon)
		return false;

	if (pWeapon->GetWeaponID() == SDK_WEAPON_BRAWL)
		return false;

	return !pWeapon->HasPrimaryAmmo();
}

bool PlayerActiveWeaponHasAimInConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	if (pPlayer->m_Shared.IsSuperFalling())
		return false;

	CWeaponSDKBase* pWeapon = pPlayer->GetActiveSDKWeapon();

	if (!pWeapon)
		return false;

	if (pWeapon->HasAimInSpeedPenalty() || pWeapon->HasAimInFireRateBonus() || pWeapon->HasAimInRecoilBonus())
		return true;

	return false;
}

bool PlayerHasGrenadesConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	return pPlayer->GetAmmoCount(GetAmmoDef()->Index("grenades"));
}

bool PlayerHasSlowMoConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	return pPlayer->GetSlowMoSeconds() > 0;
}

bool PlayerInThirdPersonConditions( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	if (!pPlayer->IsInThirdPerson())
		return false;

	return true;
}

bool PlayerSuperFalling( C_SDKPlayer *pPlayer, class CLesson *pLesson )
{
	if (!PlayerAliveConditions(pPlayer, pLesson))
		return false;

	if (!pPlayer->m_Shared.IsSuperFalling())
		return false;

	return true;
}

pfnConditionsMet CInstructor::GetBaseConditions(const CUtlString& sConditions)
{
	if (sConditions == "WhoCares")
		return WhoCaresConditions;
	else if (sConditions == "ValidPlayer")
		return ValidPlayerConditions;
	else if (sConditions == "PlayerAlive")
		return PlayerAliveConditions;
	else if (sConditions == "PlayerDead")
		return PlayerDeadConditions;
	else if (sConditions == "PlayerDeadNoKillCam")
		return PlayerDeadNoKillCamConditions;
	else if (sConditions == "PlayerCanReload")
		return PlayerCanReloadConditions;
	else if (sConditions == "PlayerOutOfAmmoAndMultipleWeapons")
		return PlayerOutOfAmmoAndMultipleWeaponsConditions;
	else if (sConditions == "PlayerHasMultipleWeapons")
		return PlayerHasMultipleWeaponsConditions;
	else if (sConditions == "PlayerActiveWeaponHasAimIn")
		return PlayerActiveWeaponHasAimInConditions;
	else if (sConditions == "PlayerHasGrenades")
		return PlayerHasGrenadesConditions;
	else if (sConditions == "PlayerHasSlowMo")
		return PlayerHasSlowMoConditions;
	else if (sConditions == "PlayerInThirdPerson")
		return PlayerInThirdPersonConditions;
	else if (sConditions == "PlayerSuperFalling")
		return PlayerSuperFalling;

	Assert(false);
	Error(std::string("Couldn't find lesson condition '").append(sConditions.Get()).append("'\n").c_str());

	return NULL;
}

pfnConditionsMet Game_GetInstructorConditions(const CUtlString& sCondition)
{
	return C_SDKPlayer::GetLocalSDKPlayer()->GetInstructor()->GetBaseConditions(sCondition);
}

void CInstructor::SetActive(bool bActive)
{
	m_bActive = bActive;
	if (!bActive)
		HideLesson();
}

void CInstructor::DisplayFirstLesson(const CUtlString& sLesson)
{
	m_bActive = true;
	m_sLastLesson = "";
	m_sCurrentLesson = sLesson;
	DisplayLesson(m_sCurrentLesson);
}

void CInstructor::NextLesson()
{
	CLesson* pLesson = GetCurrentLesson();
	if (!pLesson)
		return;

	DisplayLesson(pLesson->m_sNextLesson);
}

ConVar lesson_enable("lesson_enable", "1", FCVAR_CLIENTDLL|FCVAR_ARCHIVE, "Show player tutorials?");

void CInstructor::DisplayLesson(const CUtlString& sLesson)
{
	if (sLesson != "superfall_respawn")
	{
		if (!lesson_enable.GetBool())
			return;

		if (!m_bActive)
			return;

		// Hints are tough to read in VR, assume player already knows.
		if (UseVR())
			return;
	}

	if (sLesson.Length() == 0 || m_apLessons.Find(sLesson) == -1)
	{
		int iLesson = m_apLessons.Find(m_sCurrentLesson);
		if (m_apLessons[iLesson] && m_apLessons[iLesson]->m_bKillOnFinish)
			SetActive(false);

		HideLesson();

		return;
	}

	if (m_sCurrentLesson == sLesson)
		return;

	HideLesson();

	m_sCurrentLesson = sLesson;

	m_sLastLesson = m_sCurrentLesson;

	CLesson* pLesson = m_apLessons[m_apLessons.Find(sLesson)];

	C_SDKPlayer *pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (pLesson->m_iLearningMethod == CLesson::LEARN_DISPLAYING)
		pLocalPlayer->Instructor_LessonLearned(sLesson);

	pLocalPlayer->Instructor_LessonShowed(sLesson);

	int iLessonTrainings = pLocalPlayer->Instructor_GetLessonTrainings(sLesson);

	if (pLesson->m_sSideHintText.Length() && iLessonTrainings == 0)
	{
		CHudElement* pLessonPanel = gHUD.FindElement("CHudSideHintPanel");
		static_cast<CHudSideHintPanel*>(pLessonPanel)->SetLesson(pLesson);
		m_bSideHintShowing = true;
	}
	else
	{
		CHudElement* pLessonPanel = gHUD.FindElement("CHudLessonPanel");
		static_cast<CHudLessonPanel*>(pLessonPanel)->SetLesson(pLesson);
	}
}

void CInstructor::ShowLesson()
{
	DisplayLesson(m_sCurrentLesson);
}

void CInstructor::HideLesson()
{
	CHudElement* pLessonPanel = gHUD.FindElement("CHudLessonPanel");
	if (pLessonPanel)
		static_cast<CHudLessonPanel*>(pLessonPanel)->Reset();

	pLessonPanel = gHUD.FindElement("CHudSideHintPanel");
	if (pLessonPanel)
	{
		m_bSideHintShowing = false;
		static_cast<CHudSideHintPanel*>(pLessonPanel)->Reset();
	}

	m_sCurrentLesson = "";
}

void CInstructor::FinishedLesson(const CUtlString& sLesson, bool bForceNext)
{
	if (sLesson != m_sCurrentLesson)
		return;

	if (m_apLessons[m_apLessons.Find(sLesson)]->m_bAutoNext || bForceNext)
		NextLesson();
	else
		HideLesson();

	// If we get to the end here then we turn off the instructor as we have finished completely.
	if (GetCurrentLesson() && GetCurrentLesson()->m_bKillOnFinish)
		SetActive(false);
}

CLesson* CInstructor::GetCurrentLesson()
{
	if (!m_sCurrentLesson.Length())
		return NULL;

	return m_apLessons[m_apLessons.Find(m_sCurrentLesson.Get())];
}

static ConVar lesson_nexttime("lesson_nexttime", "20");	// Time between hints
static ConVar lesson_downtime("lesson_downtime", "70");	// Time for three other hints in the intervening time.
static ConVar lesson_learntime("lesson_learntime", "15");	// Time before a lesson can be learned again.
static ConVar lesson_debug("lesson_debug", "0");
static ConVar lesson_time("lesson_time", "8");

void C_SDKPlayer::Instructor_Initialize()
{
	m_flLastLesson = -1;

	m_apLessonPriorities.RemoveAll();

	if (m_pInstructor)
		m_pInstructor->Initialize();
}

void C_SDKPlayer::Instructor_Respawn()
{
	if (!m_pInstructor)
		return;

	if (!m_pInstructor->IsInitialized())
		m_pInstructor->Initialize();

	for (unsigned short i = m_apLessonProgress.FirstInorder(); m_apLessonProgress.IsValidIndex(i); i = m_apLessonProgress.NextInorder(i))
		m_apLessonProgress[i].m_flLastTimeLearned = 0;

	m_flLastLesson = -1;
}

void C_SDKPlayer::Instructor_Reset()
{
	m_apLessonProgress.RemoveAll();

	if (m_pInstructor)
		m_pInstructor->HideLesson();
}

typedef C_SDKPlayer::CLessonProgress* LessonProgressPointer;
bool LessonPriorityCompare( const LessonProgressPointer& l, const LessonProgressPointer& r )
{
	//C_SDKPlayer* pPlayer = Game()->GetLocalPlayer();
	//if (pPlayer->m_flLastEnemySeen && gpGlobals->curtime < pPlayer->m_flLastEnemySeen + 3 && lhs->m_bPreferNoEnemies != rhs->m_bPreferNoEnemies)
	//	return !lhs->m_bPreferNoEnemies;

	CLesson* pLessonL = C_SDKPlayer::GetLocalSDKPlayer()->GetInstructor()->GetLesson(l->m_sLessonName);
	CLesson* pLessonR = C_SDKPlayer::GetLocalSDKPlayer()->GetInstructor()->GetLesson(r->m_sLessonName);

	// If two lessons are the same priority, use the one that was taught the most amount of time ago.
	if (pLessonL->m_iPriority == pLessonR->m_iPriority)
		return l->m_flLastTimeShowed > r->m_flLastTimeShowed;

	return ( pLessonL->m_iPriority > pLessonR->m_iPriority );
}

void C_SDKPlayer::Instructor_Think()
{
	if (!m_pInstructor)
		return;

	if (!m_pInstructor->IsInitialized())
		m_pInstructor->Initialize();

	if (m_Shared.CanSuperFallRespawn())
	{
		m_pInstructor->DisplayLesson("superfall_respawn");
		return;
	}

	if (m_pInstructor->GetCurrentLesson())
	{
		if (m_pInstructor->GetCurrentLesson()->m_sLessonName == "superfall_respawn" && !m_Shared.CanSuperFallRespawn())
			m_pInstructor->HideLesson();
		else if (!IsAlive() && m_pInstructor->GetCurrentLesson()->m_pfnConditions != PlayerDeadConditions)
			m_pInstructor->HideLesson();
	}

	if (!m_apLessonProgress.Count() && m_pInstructor->GetLessons().Count())
	{
		m_apLessonProgress.SetLessFunc(Str_LessFunc);

		for (unsigned short i = m_pInstructor->GetLessons().FirstInorder(); m_pInstructor->GetLessons().IsValidIndex(i); i = m_pInstructor->GetLessons().NextInorder(i))
		{
			int iProgress = m_apLessonProgress.Insert(m_pInstructor->GetLessons().Key(i));
			CLessonProgress& oProgress = m_apLessonProgress[iProgress];
			oProgress.m_sLessonName = m_pInstructor->GetLessons().Key(i);
		}
	}

	CHudVote* pElement = dynamic_cast<CHudVote*>(gHUD.FindElement("CHudVote"));
	if (pElement && pElement->IsVoteUIActive())
		return;

	if (m_pInstructor->IsSideHintShowing())
		m_flLastLesson = gpGlobals->curtime;

	if (m_flLastLesson < 0 || gpGlobals->curtime > m_flLastLesson + lesson_nexttime.GetFloat())
	{
		m_apLessonPriorities.RemoveAll();

		for (unsigned short i = m_apLessonProgress.FirstInorder(); m_apLessonProgress.IsValidIndex(i); i = m_apLessonProgress.NextInorder(i))
		{
			CLessonProgress* pLessonProgress = &m_apLessonProgress[i];
			CLesson* pLesson = m_pInstructor->GetLesson(m_apLessonProgress.Key(i));

			if (pLesson->m_iLessonType == CLesson::LESSON_ENVIRONMENT)
				continue;

			if (!Instructor_IsLessonValid(pLessonProgress))
				continue;

			m_apLessonPriorities.Insert(pLessonProgress);
		}

		if (lesson_debug.GetBool() && m_apLessonPriorities.Count())
		{
			Msg("Instructor: Lesson priorities:\n");
			for (int j = 0; j < m_apLessonPriorities.Count(); j++)
			{
				CLesson* pLesson = m_pInstructor->GetLesson(m_apLessonPriorities[j]->m_sLessonName);
				Msg(VarArgs(const_cast<char*>(std::string(" %d - ").append(m_apLessonPriorities[j]->m_sLessonName).append(" - %d\n").c_str()), j+1, pLesson->m_iPriority));
			}
		}

		CLessonProgress* pBestLesson = Instructor_GetBestLesson();

		if (pBestLesson)
		{
			if (lesson_debug.GetBool())
			{
				CLesson* pLesson = m_pInstructor->GetLesson(pBestLesson->m_sLessonName);
				Msg(VarArgs(const_cast<char*>(std::string("Instructor: New lesson: ").append(pBestLesson->m_sLessonName).append(" Priority: %d\n").c_str()), pLesson->m_iPriority));
			}

			m_flLastLesson = gpGlobals->curtime;
			pBestLesson->m_flLastTimeShowed = gpGlobals->curtime;

			m_pInstructor->DisplayLesson(pBestLesson->m_sLessonName);
		}
		else
			// No lesson to display? Don't try again for another few seconds.
			m_flLastLesson = gpGlobals->curtime - lesson_nexttime.GetFloat() + 3;
	}
}

// http://stackoverflow.com/questions/236129/splitting-a-string-in-c
// Why doesn't the C++ STL have this sort of stuff? For that matter, why doesn't Valve?
std::vector<std::string>& split_string(const std::string &s, char delim, std::vector<std::string> &elems)
{
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

void C_SDKPlayer::Instructor_LessonLearned(const CUtlString& sLesson)
{
	if (!m_pInstructor)
		return;

	if (!m_pInstructor->IsInitialized())
		m_pInstructor->Initialize();

	int iLesson = m_apLessonProgress.Find(sLesson);
	Assert(iLesson != -1);
	if (iLesson == -1)
		return;

	CLessonProgress* pLessonProgress = &m_apLessonProgress[iLesson];

	Assert(pLessonProgress);
	if (!pLessonProgress)
		return;

	// Can only learn a lesson once in a while, to ensure that it is truly learned.
	// The idea is that the player spends a couple seconds toying around with the
	// new feature, but won't spend all of the lessons in that time.
	if (gpGlobals->curtime < pLessonProgress->m_flLastTimeLearned + lesson_learntime.GetFloat())
		return;

	pLessonProgress->m_flLastTimeLearned = gpGlobals->curtime;
	pLessonProgress->m_iTimesLearned++;

	CHudElement* pLessonPanel = gHUD.FindElement("CHudSideHintPanel");
	if (pLessonPanel)
	{
		CHudSideHintPanel* pSideHint = static_cast<CHudSideHintPanel*>(pLessonPanel);

		if (pSideHint->IsVisible() && pSideHint->GetLesson() && pSideHint->GetLesson()->m_sLessonName == sLesson)
		{
			m_pInstructor->HideLesson();

			// Play a sound?

			pSideHint->Reset();
		}
	}

	CLesson* pLesson = m_pInstructor->GetLesson(sLesson);

	if (lesson_debug.GetBool())
	{
		if (pLessonProgress->m_iTimesLearned < pLesson->m_iTimesToLearn)
			Msg(VarArgs(const_cast<char*>(std::string("Instructor: Trained lesson ").append(sLesson).append(" - %d/%d\n").c_str()), pLessonProgress->m_iTimesLearned, pLesson->m_iTimesToLearn));
		else if (pLessonProgress->m_iTimesLearned == pLesson->m_iTimesToLearn)
			Msg(std::string("Instructor: Learned lesson ").append(sLesson).append("\n").c_str());
	}

	if (pLessonProgress->m_iTimesLearned == pLesson->m_iTimesToLearn)
	{
		// Lesson has been learned. Save it to the config so that we don't show this
		// hint on consecutive boots of the game.
		std::string sLessonsLearned = da_instructor_lessons_learned.GetString();
		std::vector<std::string> asTokens;
		split_string(sLessonsLearned, ';', asTokens);

		bool bFound = false;
		for (size_t i = 0; i < asTokens.size(); i++)
		{
			if (asTokens[i] == pLesson->m_sLessonName.String())
			{
				bFound = true;
				break;
			}
		}

		if (!bFound)
			da_instructor_lessons_learned.SetValue(VarArgs("%s;%s", da_instructor_lessons_learned.GetString(), pLesson->m_sLessonName.String()));
	}
}

void C_SDKPlayer::Instructor_LessonShowed(const CUtlString& sLesson)
{
	if (!m_pInstructor)
		return;

	if (!m_pInstructor->IsInitialized())
		m_pInstructor->Initialize();

	int iLesson = m_apLessonProgress.Find(sLesson);
	Assert(iLesson != -1);
	if (iLesson == -1)
		return;

	CLessonProgress* pLessonProgress = &m_apLessonProgress[iLesson];

	Assert(pLessonProgress);
	if (!pLessonProgress)
		return;

	pLessonProgress->m_iTimesShown++;
}

bool C_SDKPlayer::Instructor_IsLessonLearned(const CLessonProgress* pLessonProgress)
{
	if (!m_pInstructor)
		return false;

	Assert(pLessonProgress);
	if (!pLessonProgress)
		return true;

	// Check the config for whether the lesson was learned in previous
	// runs of the game.
	std::string sLessonsLearned = da_instructor_lessons_learned.GetString();
	std::vector<std::string> asTokens;
	split_string(sLessonsLearned, ';', asTokens);

	for (size_t i = 0; i < asTokens.size(); i++)
	{
		if (asTokens[i] == pLessonProgress->m_sLessonName.String())
			return true;
	}

	CLesson* pLesson = m_pInstructor->GetLesson(pLessonProgress->m_sLessonName);

	Assert(pLesson);
	if (!pLesson)
		return true;

	return pLessonProgress->m_iTimesLearned >= pLesson->m_iTimesToLearn;
}

int C_SDKPlayer::Instructor_GetLessonTrainings(const CUtlString& sLesson)
{
	if (!m_pInstructor)
		return -1;

	if (!m_pInstructor->IsInitialized())
		m_pInstructor->Initialize();

	int iLesson = m_apLessonProgress.Find(sLesson);
	Assert(iLesson != m_apLessonProgress.InvalidIndex());
	if (iLesson == m_apLessonProgress.InvalidIndex())
		return 0;

	CLessonProgress* pLessonProgress = &m_apLessonProgress[iLesson];

	Assert(pLessonProgress);
	if (!pLessonProgress)
		return 0;

	return pLessonProgress->m_iTimesLearned;
}

bool C_SDKPlayer::Instructor_IsLessonValid(const CUtlString& sLesson)
{
	int iLesson = m_apLessonProgress.Find(sLesson);
	Assert(iLesson != -1);
	if (iLesson == -1)
		return false;

	return Instructor_IsLessonValid(&m_apLessonProgress[iLesson]);
}

// Can this lesson be displayed right now?
bool C_SDKPlayer::Instructor_IsLessonValid(const CLessonProgress* pLessonProgress)
{
	if (!m_pInstructor)
		return false;

	Assert(pLessonProgress);
	if (!pLessonProgress)
		return true;

	CLesson* pLesson = m_pInstructor->GetLesson(pLessonProgress->m_sLessonName);

	Assert(pLesson);
	if (!pLesson)
		return true;

	if (Instructor_IsLessonLearned(pLessonProgress))
		return false;

	if (pLessonProgress->m_flLastTimeShowed != 0 && gpGlobals->curtime < pLessonProgress->m_flLastTimeShowed + lesson_downtime.GetFloat())
		return false;

	if (pLesson->m_iMaxShows > 0 && pLessonProgress->m_iTimesShown > pLesson->m_iMaxShows)
		return false;

	for (int i = 0; i < pLesson->m_asPrerequisites.Count(); i++)
	{
		if (!Instructor_IsLessonLearned(&m_apLessonProgress[m_apLessonProgress.Find(pLesson->m_asPrerequisites[i])]))
			return false;
	}

	if (pLesson->m_pfnConditions)
		return pLesson->m_pfnConditions(this, pLesson);
	else
		return true;
}

C_SDKPlayer::CLessonProgress* C_SDKPlayer::Instructor_GetBestLesson()
{
	if (!m_apLessonPriorities.Count())
		return NULL;

	return m_apLessonPriorities[0];
}

CLesson::CLesson(CInstructor* pInstructor, const CUtlString& sLesson)
{
	m_pInstructor = pInstructor;
	m_sLessonName = sLesson;
	m_iWidth = 200;
	m_bAutoNext = true;
	m_bKillOnFinish = false;
	m_flSlideAmount = 0;
	m_bSlideX = true;
	m_iPriority = 0;
	m_iLessonType = LESSON_INFO;
	m_iLearningMethod = LEARN_DISPLAYING;
	m_iTimesToLearn = 3;
	m_pfnConditions = WhoCaresConditions;
}

void __MsgFunc_LessonLearned( bf_read &msg )
{
	char szString[256];
	msg.ReadString(szString, sizeof(szString));

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();

	pPlayer->Instructor_LessonLearned(szString);
}

DECLARE_HUDELEMENT( CHudLessonPanel );
DECLARE_HUD_MESSAGE( CHudLessonPanel, HintText );

CHudLessonPanel::CHudLessonPanel( const char *pElementName ) : BaseClass(NULL, "HudLessonPanel"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	m_pLabel = new vgui::Label( this, "HudLessonPanelLabel", "" );
	m_pLesson = NULL;
}

void CHudLessonPanel::Init()
{
	HOOK_HUD_MESSAGE( CHudLessonPanel, HintText );
	HOOK_MESSAGE( LessonLearned );

	// listen for client side events
	ListenForGameEvent( "player_hintmessage" );
}

inline std::string replace(std::string s, const std::string& f, const std::string& r)
{
	size_t iPosition;
	while ((iPosition = s.find(f)) != std::string::npos)
		s.assign(s.substr(0, iPosition).append(r).append((s.c_str()+iPosition+f.length())));

	return s;
}

void CHudLessonPanel::Reset()
{
	SetHintText( NULL );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "LessonHide" ); 
	m_bLastLabelUpdateHack = true;
}

void CHudLessonPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( GetFgColor() );
	m_hFont = pScheme->GetFont( "HudHintTextLarge", true );
	m_pLabel->SetBgColor( Color( 0, 0, 0, 150 ) );
	m_pLabel->SetPaintBackgroundType( 2 );
	m_pLabel->SetSize( 0, GetTall() );		// Start tiny, it'll grow.
}

bool CHudLessonPanel::SetHintText( wchar_t *text )
{
	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	wchar_t *p = text;

	while ( p )
	{
		wchar_t *line = p;
		wchar_t *end = wcschr( p, L'\n' );
		int linelengthbytes = 0;
		if ( end )
		{
			//*end = 0;	//eek
			p = end+1;
			linelengthbytes = ( end - line ) * 2;
		}
		else
		{
			p = NULL;
		}		

		// replace any key references with bound keys
		wchar_t buf[512];
		UTIL_ReplaceKeyBindings( line, linelengthbytes, buf, sizeof( buf ) );

		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, buf));
		label->SetFont( m_hFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}

	InvalidateLayout( true );

	return true;
}

void CHudLessonPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	int i;

	int wide, tall;
	GetSize( wide, tall );

	// find the widest line
	int iDesiredLabelWide = 0;
	for ( i=0; i < m_Labels.Count(); ++i )
	{
		iDesiredLabelWide = std::max( iDesiredLabelWide, m_Labels[i]->GetWide() );
	}

	// find the total height
	int fontTall = vgui::surface()->GetFontTall( m_hFont );
	int labelTall = fontTall * m_Labels.Count();

	iDesiredLabelWide += m_iTextX*2;
	labelTall += m_iTextY*2;

	// Now clamp it to our animation size
	iDesiredLabelWide = (iDesiredLabelWide * m_flLabelSizePercentage);

	int x, y;
	if ( m_iCenterX < 0 )
	{
		x = 0;
	}
	else if ( m_iCenterX > 0 )
	{
		x = wide - iDesiredLabelWide;
	}
	else
	{
		x = (wide - iDesiredLabelWide) / 2;
	}

	if ( m_iCenterY > 0 )
	{
		y = 0;
	}
	else if ( m_iCenterY < 0 )
	{
		y = tall - labelTall;
	}
	else
	{
		y = (tall - labelTall) / 2;
	}

	x = std::max(x,0);
	y = std::max(y,0);

	iDesiredLabelWide = std::min(iDesiredLabelWide,wide);
	m_pLabel->SetBounds( x, y, iDesiredLabelWide, labelTall );

	// now lay out the sub-labels
	for ( i=0; i<m_Labels.Count(); ++i )
	{
		int xOffset = (wide - m_Labels[i]->GetWide()) * 0.5;
		m_Labels[i]->SetPos( xOffset, y + m_iTextY + i*fontTall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudLessonPanel::OnThink()
{
	m_pLabel->SetFgColor(GetFgColor());
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->SetFgColor(GetFgColor());
	}

	// If our label size isn't at the extreme's, we're sliding open / closed
	// This is a hack to get around InvalideLayout() not getting called when
	// m_flLabelSizePercentage is changed via a HudAnimation.
	if ( m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0 || m_bLastLabelUpdateHack )
	{
		m_bLastLabelUpdateHack = (m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0);
		InvalidateLayout();
	}
}

void CHudLessonPanel::MsgFunc_HintText( bf_read &msg )
{
	// Read the string(s)
	char szString[255];
	msg.ReadString( szString, sizeof(szString) );

	char *tmpStr = hudtextmessage->LookupString( szString, NULL );
	LocalizeAndDisplay( tmpStr, szString );
}

void CHudLessonPanel::FireGameEvent( IGameEvent * event)
{
	const char *hintmessage = event->GetString( "hintmessage" );
	char *tmpStr = hudtextmessage->LookupString( hintmessage, NULL );
	LocalizeAndDisplay( tmpStr, hintmessage );
}

//-----------------------------------------------------------------------------
// Purpose: Localize, display, and animate the hud element
//-----------------------------------------------------------------------------
void CHudLessonPanel::LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString )
{
	static wchar_t szBuf[128];
	static wchar_t *pszBuf;

	// init buffers & pointers
	szBuf[0] = 0;
	pszBuf = szBuf;

	// try to localize
	if ( pszHudTxtMsg )
	{
		pszBuf = g_pVGuiLocalize->Find( pszHudTxtMsg );
	}
	else
	{
		pszBuf = g_pVGuiLocalize->Find( szRawString );
	}

	if ( !pszBuf )
	{
		// use plain ASCII string 
		g_pVGuiLocalize->ConvertANSIToUnicode( szRawString, szBuf, sizeof(szBuf) );
		pszBuf = szBuf;
	}

	// make it visible
	if ( SetHintText( pszBuf ) )
	{
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "LessonShow" ); 

		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pLocalPlayer )
		{
			pLocalPlayer->EmitSound( "Hud.Hint" );

			if ( pLocalPlayer->Hints() )
			{
				pLocalPlayer->Hints()->PlayedAHint();
			}
		}
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "LessonHide" ); 
	}
}

void CHudLessonPanel::SetLesson(CLesson* pLesson)
{
	m_pLesson = pLesson;

	LocalizeAndDisplay(pLesson->m_sText.Get(), pLesson->m_sText.Get());
}

DECLARE_HUDELEMENT( CHudSideHintPanel );

CHudSideHintPanel::CHudSideHintPanel( const char *pElementName ) : BaseClass(NULL, "HudSideHintPanel"), CHudElement( pElementName )
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent( pParent );
	SetVisible( false );
	m_pLabel = new vgui::Label( this, "HudSideHintPanelLabel", "" );
	m_pLesson = NULL;
}

void CHudSideHintPanel::Init()
{
	HOOK_MESSAGE( LessonLearned );
}

void CHudSideHintPanel::Reset()
{
	SetHintText( NULL );
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SideHintHide" ); 
	m_bLastLabelUpdateHack = true;
}

void CHudSideHintPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	SetFgColor( GetFgColor() );
	m_hFont = pScheme->GetFont( "HudHintTextLarge", true );
	m_pLabel->SetBgColor( Color( 0, 0, 0, 150 ) );
	m_pLabel->SetPaintBackgroundType( 2 );
	m_pLabel->SetSize( 0, GetTall() );		// Start tiny, it'll grow.
}

bool CHudSideHintPanel::SetHintText( wchar_t *text )
{
	// clear the existing text
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->MarkForDeletion();
	}
	m_Labels.RemoveAll();

	wchar_t *p = text;

	while ( p )
	{
		wchar_t *line = p;
		wchar_t *end = wcschr( p, L'\n' );
		int linelengthbytes = 0;
		if ( end )
		{
			//*end = 0;	//eek
			p = end+1;
			linelengthbytes = ( end - line ) * 2;
		}
		else
		{
			p = NULL;
		}

		// replace any key references with bound keys
		wchar_t buf[512];
		if (linelengthbytes)
			UTIL_ReplaceKeyBindings( line, linelengthbytes, buf, sizeof( buf ) );
		else
		{
			if (!p)
				UTIL_ReplaceKeyBindings( line, linelengthbytes, buf, sizeof( buf ) );
			else
				buf[0] = '\0';
		}

		// put it in a label
		vgui::Label *label = vgui::SETUP_PANEL(new vgui::Label(this, NULL, buf));
		label->SetFont( m_hFont );
		label->SetPaintBackgroundEnabled( false );
		label->SetPaintBorderEnabled( false );
		label->SizeToContents();
		label->SetContentAlignment( vgui::Label::a_west );
		label->SetFgColor( GetFgColor() );
		m_Labels.AddToTail( vgui::SETUP_PANEL(label) );
	}

	InvalidateLayout( true );

	return true;
}

void CHudSideHintPanel::PerformLayout()
{
	BaseClass::PerformLayout();
	int i;

	int wide, tall;
	GetSize( wide, tall );

	// find the widest line
	int iDesiredLabelWide = 0;
	for ( i=0; i < m_Labels.Count(); ++i )
	{
		iDesiredLabelWide = std::max( iDesiredLabelWide, m_Labels[i]->GetWide() );
	}

	// find the total height
	int fontTall = vgui::surface()->GetFontTall( m_hFont );
	int labelTall = fontTall * m_Labels.Count();

	iDesiredLabelWide += m_iTextX*2;
	labelTall += m_iTextY*2;

	// Now clamp it to our animation size
	iDesiredLabelWide = (iDesiredLabelWide * m_flLabelSizePercentage);

	int x, y;
	if ( m_iCenterX < 0 )
	{
		x = 0;
	}
	else if ( m_iCenterX > 0 )
	{
		x = wide - iDesiredLabelWide;
	}
	else
	{
		x = (wide - iDesiredLabelWide) / 2;
	}

	if ( m_iCenterY > 0 )
	{
		y = 0;
	}
	else if ( m_iCenterY < 0 )
	{
		y = tall - labelTall;
	}
	else
	{
		y = (tall - labelTall) / 2;
	}

	x = std::max(x,0);
	y = std::max(y,0);

	iDesiredLabelWide = std::min(iDesiredLabelWide,wide);
	m_pLabel->SetBounds( x, y, iDesiredLabelWide, labelTall );

	// now lay out the sub-labels
	for ( i=0; i<m_Labels.Count(); ++i )
	{
		m_Labels[i]->SetPos( 10, y + m_iTextY + i*fontTall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudSideHintPanel::OnThink()
{
	m_pLabel->SetFgColor(GetFgColor());
	for (int i = 0; i < m_Labels.Count(); i++)
	{
		m_Labels[i]->SetFgColor(GetFgColor());
	}

	// If our label size isn't at the extreme's, we're sliding open / closed
	// This is a hack to get around InvalideLayout() not getting called when
	// m_flLabelSizePercentage is changed via a HudAnimation.
	if ( m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0 || m_bLastLabelUpdateHack )
	{
		m_bLastLabelUpdateHack = (m_flLabelSizePercentage != 0.0 && m_flLabelSizePercentage != 1.0);
		InvalidateLayout();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Localize, display, and animate the hud element
//-----------------------------------------------------------------------------
void CHudSideHintPanel::LocalizeAndDisplay( const char *pszHudTxtMsg, const char *szRawString )
{
	static wchar_t szBuf[128];
	static wchar_t *pszBuf;

	// init buffers & pointers
	szBuf[0] = 0;
	pszBuf = szBuf;

	// try to localize
	if ( pszHudTxtMsg )
	{
		pszBuf = g_pVGuiLocalize->Find( pszHudTxtMsg );
	}
	else
	{
		pszBuf = g_pVGuiLocalize->Find( szRawString );
	}

	if ( !pszBuf )
	{
		// use plain ASCII string 
		g_pVGuiLocalize->ConvertANSIToUnicode( szRawString, szBuf, sizeof(szBuf) );
		pszBuf = szBuf;
	}

	// make it visible
	if ( SetHintText( pszBuf ) )
	{
		SetVisible( true );
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SideHintShow" ); 

		C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
		if ( pLocalPlayer )
		{
			pLocalPlayer->EmitSound( "Hud.Hint" );

			if ( pLocalPlayer->Hints() )
			{
				pLocalPlayer->Hints()->PlayedAHint();
			}
		}
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "SideHintHide" ); 
	}
}

void CHudSideHintPanel::SetLesson(CLesson* pLesson)
{
	m_pLesson = pLesson;

	LocalizeAndDisplay(pLesson->m_sSideHintText.Get(), pLesson->m_sSideHintText.Get());
}

bool CSDKPlayer::LessonPriorityLess::Less( const LessonPointer& lhs, const LessonPointer& rhs, void *pCtx )
{
	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	CInstructor* pInstructor = pPlayer->GetInstructor();

	//if (pPlayer->m_flLastEnemySeen && gpGlobals->curtime < pPlayer->m_flLastEnemySeen + 3 && lhs->m_bPreferNoEnemies != rhs->m_bPreferNoEnemies)
	//	return !lhs->m_bPreferNoEnemies;

	return ( pInstructor->GetLesson(lhs->m_sLessonName)->m_iPriority < pInstructor->GetLesson(rhs->m_sLessonName)->m_iPriority );
}

void CC_ResetLessons()
{
	da_instructor_lessons_learned.SetValue("");

	C_SDKPlayer* pPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	if (!pPlayer)
		return;

	pPlayer->Instructor_Initialize();
	pPlayer->Instructor_Reset();
}

static ConCommand lesson_reset("lesson_reset", CC_ResetLessons, "Reset the game instructor lessons.");
