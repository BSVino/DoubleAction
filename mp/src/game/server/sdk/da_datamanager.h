#pragma once

#ifdef WITH_DATA_COLLECTION

#include "vstdlib/jobthread.h"

#include "sdk_shareddefs.h"
#include "sdk_player.h"

namespace da
{
	namespace protobuf
	{
		class GameData;
		class KillInfo;
	}
}

class CDataManager : public CAutoGameSystemPerFrame
{
public:
	CDataManager( char const *name );
	virtual ~CDataManager();

public:
	virtual void LevelInitPostEntity();
	virtual void FrameUpdatePostEntityThink();
	virtual void LevelShutdownPostEntity();

	virtual void SavePositions();

	void AddKillInfo(const CTakeDamageInfo& info, CSDKPlayer* pKilled);

	void AddCharacterChosen(const char* pszCharacter);
	void AddWeaponChosen(SDKWeaponID eWeapon);
	void AddSkillChosen(SkillID eSkill);

	void ClientConnected(AccountID_t eAccountID);
	void ClientDisconnected(AccountID_t eAccountID);

	void SetTeamplay(bool bOn);

	void VotePassed(const char* pszIssue, const char* pszDetails);
	void VoteFailed(const char* pszIssue);

	bool IsSendingData();

	void FillProtoBuffer(da::protobuf::GameData* pbGameData);
	void ClearData();

private:
	class CDataContainer
	{
	private:
		static bool Str_LessFunc( CUtlString const &a, CUtlString const &b )
		{
			return strcmp(a.Get(), b.Get()) < 0;
		}

	public:
		CDataContainer()
		{
			memset(&z, 0, sizeof(z));

			m_asCharactersChosen.SetLessFunc(Str_LessFunc);
		}

		~CDataContainer();

		struct
		{
			float m_flStartTime;
			bool  m_bTeamplay;
			float m_flNextPositionsUpdate;
			int   m_iConnections;
			int   m_iDisconnections;
			int   m_iUniquePlayers;
			int   m_iThirdPersonActive;
			int   m_iThirdPersonInactive;
			int   m_iVRActive;
			int   m_iVRInactive;
			int   m_iWindows;
			int   m_iLinux;
			int   m_iMac;
			bool  m_bCheated;
		} z; // Stuff that needs to be initialized to 0

		CUtlVector<Vector> m_avecPlayerPositions;
		CUtlMap<CUtlString, int> m_asCharactersChosen;
		CUtlVector<SDKWeaponID> m_aeWeaponsChosen;
		CUtlVector<SkillID>     m_aeSkillsChosen;
		struct VoteResult
		{
			CUtlString  m_sIssue;
			CUtlString  m_sDetails;
			bool        m_bResult;
		};
		CUtlVector<VoteResult> m_aVoteResults;
		CUtlVector<class da::protobuf::KillInfo*> m_apKillInfos;
	}* d;

	// We use this to figure out if a client has really connected
	// so it needs to be outside the data container so it doesn't get wiped.
	CUtlMap<AccountID_t, char> m_aiConnectedClients;

	bool  m_bLevelStarted;
	CJob* m_pSendData;
};

CDataManager& DataManager();

#endif
