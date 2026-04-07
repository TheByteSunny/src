/*
  just header file for cbaseflex bind attempt
*/

#pragma once

extern "C" {
#include "lua.h"
#include "lauxlib.h"
}

class CBaseFlex;

LUA_API CBaseFlex *lua_toflex(lua_State *L, int idx);

LUA_API void lua_pushflex(lua_State *L, CBaseFlex *pEntity);

LUALIB_API CBaseFlex *luaL_checkflex(lua_State *L, int narg);

LUALIB_API int luaopen_CBaseFlex(lua_State *L);
