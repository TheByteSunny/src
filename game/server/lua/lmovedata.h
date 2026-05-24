//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#ifndef LMOVEDATA_H
#define LMOVEDATA_H

#ifdef _WIN32
#pragma once
#endif

#include "lua.h"
#include "lauxlib.h"
#include "luasrclib.h"
#include "igamemovement.h"

LUA_API void       lua_pushmovedata(lua_State *L, CMoveData *mv);
LUA_API CMoveData *lua_tomovedata(lua_State *L, int idx);
LUA_API CMoveData *luaL_checkmovedata(lua_State *L, int narg);

#endif // LMOVEDATA_H