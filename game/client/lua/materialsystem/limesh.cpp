//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "materialsystem/imaterialsystem.h"
#include "limesh.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

LUALIB_API lua_IMesh *luaL_checkimesh( lua_State *L, int idx )
{
	lua_MeshRef *ref = (lua_MeshRef *)luaL_checkudata( L, idx, LUA_IMESHLIBNAME );
	if ( !ref || !ref->m_pMesh )
		luaL_error( L, "IMesh is null" );
	return ref->m_pMesh;
}

/*
** push functions (C -> stack)
*/

LUA_API void lua_pushimesh( lua_State *L, lua_IMesh *pMesh, bool bOwned )
{
	if ( !pMesh )
	{
		lua_pushnil( L );
		return;
	}
	lua_MeshRef *ref = (lua_MeshRef *)lua_newuserdata( L, sizeof( lua_MeshRef ) );
	ref->m_pMesh = pMesh;
	ref->m_bOwned = bOwned;
	luaL_getmetatable( L, LUA_IMESHLIBNAME );
	lua_setmetatable( L, -2 );
}


static int IMesh_Draw( lua_State *L )
{
	lua_IMesh *m = luaL_checkimesh( L, 1 );
	int		   firstIndex = luaL_optint( L, 2, -1 );
	int		   numIndices = luaL_optint( L, 3, 0 );
	m->Draw( firstIndex, numIndices );
	return 0;
}

static int IMesh_SetPrimitiveType( lua_State *L )
{
	lua_IMesh *m = luaL_checkimesh( L, 1 );
	int		   type = luaL_checkint( L, 2 );
	m->SetPrimitiveType( (MaterialPrimitiveType_t)type );
	return 0;
}

static int IMesh_VertexCount( lua_State *L )
{
	lua_IMesh *m = luaL_checkimesh( L, 1 );
	lua_pushinteger( L, m->VertexCount() );
	return 1;
}

static int IMesh_IndexCount( lua_State *L )
{
	lua_IMesh *m = luaL_checkimesh( L, 1 );
	lua_pushinteger( L, m->IndexCount() );
	return 1;
}

static int IMesh_Destroy( lua_State *L )
{
	lua_MeshRef *ref = (lua_MeshRef *)luaL_checkudata( L, 1, LUA_IMESHLIBNAME );
	if ( ref && ref->m_pMesh && ref->m_bOwned )
	{
		CMatRenderContextPtr pRC( materials );
		pRC->DestroyStaticMesh( ref->m_pMesh );
		ref->m_pMesh = NULL;
		ref->m_bOwned = false;
	}
	return 0;
}

static int IMesh_gc( lua_State *L )
{
	lua_MeshRef *ref = (lua_MeshRef *)luaL_checkudata( L, 1, LUA_IMESHLIBNAME );
	if ( ref && ref->m_pMesh && ref->m_bOwned )
	{
		CMatRenderContextPtr pRC( materials );
		pRC->DestroyStaticMesh( ref->m_pMesh );
		ref->m_pMesh = NULL;
	}
	return 0;
}

static int IMesh_tostring( lua_State *L )
{
	lua_MeshRef *ref = (lua_MeshRef *)luaL_checkudata( L, 1, LUA_IMESHLIBNAME );
	lua_pushfstring( L, "IMesh: %p (%s)", ref->m_pMesh, ref->m_bOwned ? "static" : "dynamic" );
	return 1;
}

static int IMesh_eq( lua_State *L )
{
	lua_MeshRef *a = (lua_MeshRef *)luaL_checkudata( L, 1, LUA_IMESHLIBNAME );
	lua_MeshRef *b = (lua_MeshRef *)luaL_checkudata( L, 2, LUA_IMESHLIBNAME );
	lua_pushboolean( L, a->m_pMesh == b->m_pMesh );
	return 1;
}

static const luaL_Reg IMesh_methods[] = {
	{"Draw",             IMesh_Draw},
	{"SetPrimitiveType", IMesh_SetPrimitiveType},
	{"VertexCount",      IMesh_VertexCount},
	{"IndexCount",       IMesh_IndexCount},
	{"Destroy",          IMesh_Destroy},

	{NULL, NULL}
};

/*
** Open IMesh object
*/
LUALIB_API int luaopen_IMesh( lua_State *L )
{
	luaL_newmetatable( L, LUA_IMESHLIBNAME );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pushcfunction( L, IMesh_tostring );
	lua_setfield( L, -2, "__tostring" );

	lua_pushcfunction( L, IMesh_eq );
	lua_setfield( L, -2, "__eq" );

	lua_pushcfunction( L, IMesh_gc );
	lua_setfield( L, -2, "__gc" );

	luaL_register( L, NULL, IMesh_methods );

	lua_pop( L, 1 );
	return 0;
}
