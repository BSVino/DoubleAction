#pragma once

#ifdef WITH_DATA_COLLECTION

#include "vstdlib/jobthread.h"

#include "sdk_shareddefs.h"

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

public:
	virtual void LevelInitPostEntity();
	virtual void FrameUpdatePostEntityThink();
	virtual void LevelShutdownPostEntity();

	virtual void SavePositions();

	void AddCharacterChosen(const char* pszCharacter);
	void AddWeaponChosen(SDKWeaponID eWeapon);
	void AddSkillChosen(SkillID eSkill);

	bool IsSendingData();

	void FillProtoBuffer(da::protobuf::GameData* pbGameData);
	void ClearData();

private:
	float              m_flNextPositionsUpdate;
	CUtlVector<Vector> m_avecPlayerPositions;
	CUtlVector<const char*> m_apszCharactersChosen;
	CUtlVector<SDKWeaponID> m_aeWeaponsChosen;
	CUtlVector<SkillID>     m_aeSkillsChosen;

	bool               m_bCheated;

	CJob* m_pSendData;
};

CDataManager& DataManager();

#endif
