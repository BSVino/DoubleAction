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
public:
	slowmo_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bWITH_BLUR = true;
#endif // _DEBUG
		m_nWITH_BLUR =  0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bWITH_BLUR;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nWITH_BLUR ) + 0;
	}
};
#define shaderStaticTest_slowmo_ps20b 0
class slowmo_ps20b_Dynamic_Index
{
public:
	slowmo_ps20b_Dynamic_Index()
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderDynamicTest_slowmo_ps20b 0
