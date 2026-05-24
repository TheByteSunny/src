//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LAMMODEF_H
#define LAMMODEF_H
#ifdef _WIN32
#pragma once
#endif

/* type for CAmmoDef functions */
typedef CAmmoDef lua_CAmmoDef;


/*
** access functions (stack -> C)
*/

LUA_API lua_CAmmoDef* lua_toammodef(lua_State* L, int idx);

LUALIB_API lua_CAmmoDef* luaL_checkammodef(lua_State* L, int narg);


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushammodef(lua_State* L, lua_CAmmoDef* pAmmoDef);

#endif