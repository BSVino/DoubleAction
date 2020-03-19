#include "cbase.h"
#include "c_func_break.h"


LINK_ENTITY_TO_CLASS(func_breakable, C_Breakable);

IMPLEMENT_CLIENTCLASS_DT(C_Breakable, DT_Breakable, CBreakable)
RecvPropInt(RECVINFO(m_iHealth)),
END_RECV_TABLE()
