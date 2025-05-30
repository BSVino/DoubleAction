#pragma once

class CHealthWidget
{
public:
	void SetPlayer(CSDKPlayer* pPlayer);
	void Update();
	void Paint(CHudTexture* pIcon, float flIconHeight, int x, int y, int iWidth, int iHeight, float flAlpha);

	float GetLerpedHealth() const;

private:
	CHandle<CSDKPlayer> m_hPlayer;
	int m_iOldHealth = 0;
	int m_iHealth = 0;
	float m_flHealthLerpTime = 0.25f;
	float m_flLastHealthChange = 0.0f;
};
