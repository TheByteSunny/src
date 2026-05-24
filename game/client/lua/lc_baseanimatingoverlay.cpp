//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "lc_baseanimatingoverlay.h"
#include "c_baseanimatingoverlay.h"
#include "lc_baseanimating.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LUA_API lua_CBaseAnimatingOverlay *lua_toanimatingoverlay( lua_State *L, int idx )
{
	CBaseHandle *hEntity = dynamic_cast< CBaseHandle * >( (CBaseHandle *)lua_touserdata( L, idx ) );
	if ( hEntity == NULL )
		return NULL;
	return dynamic_cast< lua_CBaseAnimatingOverlay * >( hEntity->Get() );
}

LUA_API void lua_pushanimatingoverlay( lua_State *L, C_BaseAnimatingOverlay *pEntity )
{
	CBaseHandle *hEntity = (CBaseHandle *)lua_newuserdata( L, sizeof( CBaseHandle ) );
	hEntity->Set( pEntity );
	luaL_getmetatable( L, "CBaseAnimatingOverlay" );
	lua_setmetatable( L, -2 );
}

LUALIB_API lua_CBaseAnimatingOverlay *luaL_checkanimatingoverlay( lua_State *L, int narg )
{
	lua_CBaseAnimatingOverlay *d = lua_toanimatingoverlay( L, narg );
	if ( d == NULL )
		luaL_argerror( L, narg, "CBaseAnimatingOverlay expected, got NULL entity" );
	return d;
}

LUALIB_API int luaopen_CBaseAnimatingOverlay( lua_State *L )
{
    luaL_newmetatable( L, "CBaseAnimatingOverlay" );

    luaL_getmetatable( L, "CBaseAnimating" );
    if ( lua_istable( L, -1 ) )
        lua_setfield( L, -2, "__index" );
    else
        lua_pop( L, 1 );

    lua_pushstring( L, "entity" );
    lua_setfield( L, -2, "__type" );

    lua_pop( L, 1 );
    return 1;
}
