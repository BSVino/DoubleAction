#pragma once

#ifdef WITH_DATA_COLLECTION

#include "vstdlib/jobthread.h"

#include "sdk_shareddefs.h"

#include <map>

namespace da
{
	namespace protobuf
	{
		class GameData;
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

	void AddPlayerKill(const Vector& vecPosition);
	void AddPlayerDeath(const Vector& vecPosition);

	void AddCharacterChosen(const char* pszCharacter);
	void AddWeaponChosen(SDKWeaponID eWeapon);
	void AddSkillChosen(SkillID eSkill);

	void ClientConnected(AccountID_t eAccountID);
	void ClientDisconnected(AccountID_t eAccountID);

	bool IsSendingData();

	void FillProtoBuffer(da::protobuf::GameData* pbGameData);
	void ClearData();

private:
	class CDataContainer
	{
	public:
		CDataContainer()
		{
			m_flNextPositionsUpdate = 0;

			m_iConnections = 0;

			m_bCheated = false;
		}

		float              m_flNextPositionsUpdate;
		CUtlVector<Vector> m_avecPlayerPositions;
		CUtlVector<Vector> m_avecPlayerKills;
		CUtlVector<Vector> m_avecPlayerDeaths;
		std::map<std::string, int> m_asCharactersChosen;
		CUtlVector<SDKWeaponID> m_aeWeaponsChosen;
		CUtlVector<SkillID>     m_aeSkillsChosen;
		int                m_iConnections;

		bool               m_bCheated;
	}* d;

	// We use this to figure out if a client has really connected
	// so it needs to be outside the data container so it doesn't get wiped.
	std::map<AccountID_t, char> m_aiConnectedClients;

	bool  m_bLevelStarted;
	CJob* m_pSendData;
};

CDataManager& DataManager();

#endif
