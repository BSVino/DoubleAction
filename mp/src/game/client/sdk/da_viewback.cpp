#include "cbase.h"

using namespace std;

#ifdef WITH_VIEWBACK

#include "da_viewback.h"

#include "../../../../viewback/server/viewback_util.h"

#include "convar.h"

#include "c_sdk_player.h"

#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef WITH_VIEWBACK

CViewbackSystem g_Viewback( "CViewbackSystem" );

CViewbackSystem& ViewbackSystem()
{
	return g_Viewback;
}

CViewbackSystem::CViewbackSystem( char const* name )
	: CAutoGameSystemPerFrame(name)
{
}

CViewbackSystem::~CViewbackSystem()
{
}

static void viewback_debug_output(const char* text)
{
	Msg("VB: %s\n", text);
}

static void viewback_console_command(const char* text)
{
	engine->ClientCmd_Unrestricted(text);
}

void CViewbackSystem::LevelInitPostEntity( void )
{
	if (vb_server_is_active())
		vb_server_shutdown();

	vb_util_initialize();

	vb_util_add_channel("Style", VB_DATATYPE_FLOAT, &m_ePlayerStyle);
	vb_util_add_channel("AimIn", VB_DATATYPE_FLOAT, &m_eAimIn);
	vb_util_add_channel("SlowMo", VB_DATATYPE_FLOAT, &m_eSlowMo);
	vb_util_add_channel("AnimAim", VB_DATATYPE_VECTOR, &m_eAnimAim);
	vb_util_add_channel("AnimMoveYaw", VB_DATATYPE_VECTOR, &m_eAnimMoveYaw);
	vb_util_add_channel("PlayerX", VB_DATATYPE_FLOAT, &m_ePlayerOriginX);
	vb_util_add_channel("PlayerY", VB_DATATYPE_FLOAT, &m_ePlayerOriginY);
	vb_util_add_channel("PlayerZ", VB_DATATYPE_FLOAT, &m_ePlayerOriginZ);
	vb_util_add_channel("PlayerSpeed", VB_DATATYPE_FLOAT, &m_ePlayerSpeed);
	vb_util_add_channel("PlayerMaxSpeed", VB_DATATYPE_FLOAT, &m_ePlayerMaxSpeed);
	vb_util_add_channel("Recoil", VB_DATATYPE_VECTOR, &m_ePlayerRecoil);
	vb_util_add_channel("Recoil", VB_DATATYPE_FLOAT, &m_ePlayerRecoilFloat);
	vb_util_add_channel("ViewPunch", VB_DATATYPE_FLOAT, &m_ePlayerViewPunch);

	vb_group_handle_t eInstructorGroup;
	vb_util_add_group("Instructor", &eInstructorGroup);

	C_SDKPlayer* pLocalPlayer = C_SDKPlayer::GetLocalSDKPlayer();
	CInstructor* pInstructor = pLocalPlayer->GetInstructor();
	auto& aLessons = pInstructor->GetLessons();
	m_aeLessons.EnsureCount(aLessons.Count());
	for (unsigned int i = 0; i < aLessons.Count(); i++)
	{
		vb_util_add_channel(aLessons[i]->m_sLessonName.Get(), VB_DATATYPE_INT, &m_aeLessons[i]);
		vb_util_add_channel_to_group(eInstructorGroup, m_aeLessons[i]);
		vb_util_add_label(m_aeLessons[i], aLessons[i]->m_iTimesToLearn, "Learned");
	}

	vb_group_handle_t eDAGroup, eAnimationGroup, eMovementGroup, eRecoilGroup;

	vb_util_add_group("DoubleAction", &eDAGroup);
	vb_util_add_group("Animation", &eAnimationGroup);
	vb_util_add_group("Recoil", &eRecoilGroup);
	vb_util_add_group("Movement", &eMovementGroup);

	vb_util_add_channel_to_group(eRecoilGroup, m_ePlayerRecoil);
	vb_util_add_channel_to_group(eRecoilGroup, m_ePlayerRecoilFloat);
	vb_util_add_channel_to_group(eRecoilGroup, m_ePlayerViewPunch);

	if (!vb_util_add_channel_to_group_s("DoubleAction", "Style") ||
		!vb_util_add_channel_to_group_s("DoubleAction", "AimIn") ||
		!vb_util_add_channel_to_group_s("DoubleAction", "SlowMo") ||
		!vb_util_add_channel_to_group_s("Animation", "AnimAim") ||
		!vb_util_add_channel_to_group_s("Animation", "AnimMoveYaw") ||
		!vb_util_add_channel_to_group_s("Movement", "PlayerX") ||
		!vb_util_add_channel_to_group_s("Movement", "PlayerY") ||
		!vb_util_add_channel_to_group_s("Movement", "PlayerZ") ||
		!vb_util_add_channel_to_group_s("Movement", "PlayerSpeed") ||
		!vb_util_add_channel_to_group_s("Movement", "PlayerMaxSpeed"))
	{
		Msg("Viewback: Couldn't set up groups\n");
		return;
	}

	ConVarRef da_stylemeteractivationcost("da_stylemeteractivationcost");
	vb_util_set_range(m_ePlayerStyle, 0, da_stylemeteractivationcost.GetFloat());
	vb_util_set_range(m_eAimIn, 0, 1);
	vb_util_set_range(m_ePlayerSpeed, 0, 500);
	vb_util_set_range(m_ePlayerMaxSpeed, 0, 500);

	vb_util_set_max_connections(2);
	vb_util_set_output_callback(viewback_debug_output);
	vb_util_set_command_callback(viewback_console_command);

	m_sServerName = std::string("Double Action: ") + C_BasePlayer::GetLocalPlayer()->GetPlayerName();

	if (!vb_util_server_create(m_sServerName.c_str()))
	{
		Msg("Viewback: Couldn't create Viewback server\n");
		return;
	}

	// Initialize
	for (unsigned int i = 0; i < aLessons.Count(); i++)
		vb_data_send_int_s(aLessons[i]->m_sLessonName, 0);

	g_pCVar->InstallConsoleDisplayFunc( this );

	Msg("Viewback: Viewback server running.\n");
}

void CViewbackSystem::Update( float frametime )
{
	vb_server_update(gpGlobals->curtime);
}

void CViewbackSystem::LevelShutdownPostEntity()
{
	Msg("Viewback: Viewback server shut down.\n");

	g_pCVar->RemoveConsoleDisplayFunc( this );

	vb_server_shutdown();
}

void CViewbackSystem::ColorPrint( const Color& clr, const char *msg )
{
	vb_console_append(msg);
}

void CViewbackSystem::Print(const char *msg)
{
	vb_console_append(msg);
}

void CViewbackSystem::DPrint( const char *msg )
{
	vb_console_append(msg);
}

#endif
