//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lc_baseflex.h"
#include "c_baseflex.h"
#include "lc_baseanimating.h"
#include "mathlib/lvector.h"
#include "lvphysics_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

LUA_API lua_CBaseFlex *lua_toflex( lua_State *L, int idx )
{
	CBaseHandle *hEntity = dynamic_cast< CBaseHandle * >( (CBaseHandle *)lua_touserdata( L, idx ) );
	if ( hEntity == NULL )
		return NULL;
	return dynamic_cast< lua_CBaseFlex * >( hEntity->Get() );
}

/*
** push functions (C -> stack)
*/

LUA_API void lua_pushflex( lua_State *L, C_BaseFlex *pEntity )
{
	CBaseHandle *hEntity = (CBaseHandle *)lua_newuserdata( L, sizeof( CBaseHandle ) );
	hEntity->Set( pEntity );
	luaL_getmetatable( L, "CBaseFlex" );
	lua_setmetatable( L, -2 );
}

LUALIB_API lua_CBaseFlex *luaL_checkflex( lua_State *L, int narg )
{
	lua_CBaseFlex *d = lua_toflex( L, narg );
	if ( d == NULL )
		luaL_argerror( L, narg, "CBaseFlex expected, got NULL entity" );
	return d;
}

LUALIB_API int luaopen_CBaseFlex( lua_State *L )
{
    luaL_newmetatable( L, "CBaseFlex" );

    luaL_getmetatable( L, "CBaseAnimatingOverlay" );
    if ( lua_istable( L, -1 ) )
        lua_setfield( L, -2, "__index" );
    else
        lua_pop( L, 1 );

    lua_pushstring( L, "entity" );
    lua_setfield( L, -2, "__type" );

    lua_pop( L, 1 );
    return 1;
}
