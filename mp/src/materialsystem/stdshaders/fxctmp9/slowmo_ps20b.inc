#include "shaderlib/cshader.h"
class slowmo_ps20b_Static_Index
{
private:
	int m_nWITH_BLUR;
#ifdef _DEBUG
	bool m_bWITH_BLUR;
#endif
public:
	void SetWITH_BLUR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWITH_BLUR = i;
#ifdef _DEBUG
		m_bWITH_BLUR = true;
#endif
	}
	void SetWITH_BLUR( bool i )
	{
		m_nWITH_BLUR = i ? 1 : 0;
#ifdef _DEBUG
		m_bWITH_BLUR = true;
#endif
	}
private:
	int m_nWITH_OSX;
#ifdef _DEBUG
	bool m_bWITH_OSX;
#endif
public:
	void SetWITH_OSX( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWITH_OSX = i;
#ifdef _DEBUG
		m_bWITH_OSX = true;
#endif
	}
	void SetWITH_OSX( bool i )
	{
		m_nWITH_OSX = i ? 1 : 0;
#ifdef _DEBUG
		m_bWITH_OSX = true;
#endif
	}
public:
	slowmo_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bWITH_BLUR = true;
#endif // _DEBUG
		m_nWITH_BLUR =  0;
#ifdef _DEBUG
		m_bWITH_OSX = true;
#endif // _DEBUG
		m_nWITH_OSX =  0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bWITH_BLUR && m_bWITH_OSX;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 2 * m_nWITH_BLUR ) + ( 4 * m_nWITH_OSX ) + 0;
	}
};
#define shaderStaticTest_slowmo_ps20b 0
class slowmo_ps20b_Dynamic_Index
{
private:
	int m_nWITH_VR;
#ifdef _DEBUG
	bool m_bWITH_VR;
#endif
public:
	void SetWITH_VR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWITH_VR = i;
#ifdef _DEBUG
		m_bWITH_VR = true;
#endif
	}
	void SetWITH_VR( bool i )
	{
		m_nWITH_VR = i ? 1 : 0;
#ifdef _DEBUG
		m_bWITH_VR = true;
#endif
	}
public:
	slowmo_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bWITH_VR = false;
#endif // _DEBUG
		m_nWITH_VR = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bWITH_VR;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWITH_VR ) + 0;
	}
};
#define shaderDynamicTest_slowmo_ps20b psh_forgot_to_set_dynamic_WITH_VR + 0
