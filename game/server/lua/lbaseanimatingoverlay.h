//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LBASEANIMATINGOVERLAY_H
#define LBASEANIMATINGOVERLAY_H
#ifdef _WIN32
#pragma once
#endif

/* type for CBaseAnimatingOverlay functions */
typedef CBaseAnimatingOverlay lua_CBaseAnimatingOverlay;

/*
** access functions (stack -> C)
*/

LUA_API lua_CBaseAnimatingOverlay *(lua_toanimatingoverlay)( lua_State * L, int idx );

/*
** push functions (C -> stack)
*/
LUA_API void( lua_pushanimatingoverlay )( lua_State *L, lua_CBaseAnimatingOverlay *pEntity );

LUALIB_API lua_CBaseAnimatingOverlay *(luaL_checkanimatingoverlay)( lua_State * L, int narg );

#endif // LBASEANIMATINGOVERLAY_H