
#include "GameEventListener.h"
#include <igamesystem.h>

class DA_AchievementListener : public CGameEventListener, public CBaseGameSystem
{

public:
	DA_AchievementListener();
	virtual ~DA_AchievementListener();

public: // IGameEventListener Interface
	virtual void FireGameEvent(IGameEvent *event);

public: // CBaseGameSystem overrides
	virtual bool Init();
	virtual void Shutdown();

protected:
	virtual void HandleEvent(IGameEvent *event);
};