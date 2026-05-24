//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//
#ifndef LMATRENDERCONTEXT_H
#define LMATRENDERCONTEXT_H
#ifdef _WIN32
#pragma once
#endif

/* type for MatRenderContext functions */
struct lua_MatRenderContextRef
{
	IMatRenderContext *m_pContext;
	bool               m_bOwnsRef;
};

/*
** access functions (stack -> C)
*/

LUALIB_API lua_MatRenderContextRef *luaL_checkmatrendercontextref( lua_State *L, int idx );
LUALIB_API IMatRenderContext       *luaL_checkmatrendercontext   ( lua_State *L, int idx );


/*
** push functions (C -> stack)
*/

LUALIB_API void lua_pushmatrendercontext( lua_State *L, IMatRenderContext *pCtx );

#endif // LMATRENDERCONTEXT_H