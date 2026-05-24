//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#ifndef LIMESH_H
#define LIMESH_H
#ifdef _WIN32
#pragma once
#endif

/* type for IMesh functions */
typedef IMesh lua_IMesh;

struct lua_MeshRef
{
	IMesh *m_pMesh;
	bool   m_bOwned;
};


/*
** access functions (stack -> C)
*/

LUALIB_API lua_IMesh *luaL_checkimesh( lua_State *L, int idx );


/*
** push functions (C -> stack)
*/

LUA_API void       lua_pushimesh( lua_State *L, lua_IMesh *pMesh, bool bOwned );

#endif // LIMESH_H