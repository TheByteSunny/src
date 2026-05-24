//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnextbotbehaviour.h"

ActionResult< CLuaNextBot > CLuaBehaviour::OnStart( CLuaNextBot *me, Action< CLuaNextBot > *priorAction )
{
	me->CallLuaHook( "BehaveStart" );
	return Continue();
}

ActionResult< CLuaNextBot > CLuaBehaviour::Update( CLuaNextBot *me, float interval )
{
	me->CallLuaHook( "BehaveAct" );
	me->CallLuaHook( "Think" );
	return Continue();
}
