//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#ifndef LMESHBUILDER_H
#define LMESHBUILDER_H
#ifdef _WIN32
#pragma once
#endif

struct lua_MeshBuilderRef
{
	CMeshBuilder *m_pBuilder;
	bool          m_bBuilding;
};

/*
** access functions (stack -> C)
*/

LUALIB_API lua_MeshBuilderRef *luaL_checkmeshbuilder( lua_State *L, int idx, const char *fn );

LUALIB_API lua_MeshBuilderRef *luaL_checkmeshbuilderref( lua_State *L, int idx );

/*
** push functions (C -> stack)
*/


#endif // LMESHBUILDER_H