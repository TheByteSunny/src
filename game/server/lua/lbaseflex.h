//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LBASEFLEX_H
#define LBASEFLEX_H
#ifdef _WIN32
#pragma once
#endif

/* type for CBaseFlex functions */
typedef CBaseFlex lua_CBaseFlex;

/*
** access functions (stack -> C)
*/

LUA_API lua_CBaseFlex *(lua_toflex)( lua_State * L, int idx );

/*
** push functions (C -> stack)
*/
LUA_API void( lua_pushflex )( lua_State *L, lua_CBaseFlex *pEntity );

LUALIB_API lua_CBaseFlex *(luaL_checkflex)( lua_State * L, int narg );

#endif // LBASEFLEX_H