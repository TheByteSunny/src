//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose: Lua bindings for IMaterialVar.
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "lmaterialvar.h"
#include "materialsystem/limaterial.h"
#include "litexture.h"
#include "mathlib/lvector.h"
#include "mathlib/lvmatrix.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

LUALIB_API IMaterialVar *luaL_checkmaterialvar( lua_State *L, int idx )
{
	IMaterialVar **pp =
		(IMaterialVar **)luaL_checkudata( L, idx, LUA_MATERIALVARLIBNAME );
	if ( !pp || !*pp )
		luaL_error( L, "IMaterialVar is invalid" );
	return *pp;
}

/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmaterialvar( lua_State *L, IMaterialVar *pVar )
{
	if ( !pVar )
	{
		lua_pushnil( L );
		return;
	}

	IMaterialVar **pp =
		(IMaterialVar **)lua_newuserdata( L, sizeof( IMaterialVar * ) );
	*pp = pVar;

	luaL_getmetatable( L, LUA_MATERIALVARLIBNAME );
	lua_setmetatable( L, -2 );
}


static int IMaterialVar_GetName( lua_State *L )
{
	lua_pushstring( L, luaL_checkmaterialvar( L, 1 )->GetName() );
	return 1;
}

static int IMaterialVar_GetType( lua_State *L )
{
	lua_pushinteger( L, (int)luaL_checkmaterialvar( L, 1 )->GetType() );
	return 1;
}

static int IMaterialVar_IsDefined( lua_State *L )
{
	lua_pushboolean( L, luaL_checkmaterialvar( L, 1 )->IsDefined() );
	return 1;
}

static int IMaterialVar_SetUndefined( lua_State *L )
{
	luaL_checkmaterialvar( L, 1 )->SetUndefined();
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetFloatValue( lua_State *L )
{
	lua_pushnumber( L, luaL_checkmaterialvar( L, 1 )->GetFloatValue() );
	return 1;
}

static int IMaterialVar_SetFloatValue( lua_State *L )
{
	luaL_checkmaterialvar( L, 1 )->SetFloatValue( (float)luaL_checknumber( L, 2 ) );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetIntValue( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmaterialvar( L, 1 )->GetIntValue() );
	return 1;
}

static int IMaterialVar_SetIntValue( lua_State *L )
{
	luaL_checkmaterialvar( L, 1 )->SetIntValue( luaL_checkint( L, 2 ) );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetStringValue( lua_State *L )
{
	lua_pushstring( L, luaL_checkmaterialvar( L, 1 )->GetStringValue() );
	return 1;
}

static int IMaterialVar_SetStringValue( lua_State *L )
{
	luaL_checkmaterialvar( L, 1 )->SetStringValue( luaL_checkstring( L, 2 ) );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetVecValue( lua_State *L )
{
	IMaterialVar *pVar  = luaL_checkmaterialvar( L, 1 );
	int           count = pVar->VectorSize();

	float v[4] = { 0.f, 0.f, 0.f, 0.f };
	pVar->GetVecValue( v, count );

	for ( int i = 0; i < count; ++i )
		lua_pushnumber( L, v[i] );
	return count;
}

static int IMaterialVar_SetVecValue( lua_State *L )
{
	IMaterialVar *pVar = luaL_checkmaterialvar( L, 1 );
	int top = lua_gettop( L );

	if ( top == 2 && lua_isuserdata( L, 2 ) )
	{
		Vector vec = luaL_checkvector( L, 2 );
		pVar->SetVecValue( vec.x, vec.y, vec.z );
		lua_settop( L, 1 );
		return 1;
	}

	// numeric components
	int n = top - 1;
	if ( n < 2 || n > 4 )
		return luaL_error( L, "SetVecValue expects 2-4 numeric components or a Vector" );

	float v[4] = { 0.f, 0.f, 0.f, 0.f };
	for ( int i = 0; i < n; ++i )
		v[i] = (float)luaL_checknumber( L, 2 + i );

	pVar->SetVecValue( v, n );
	lua_settop( L, 1 );
	return 1;
}

static int IMaterialVar_VectorSize( lua_State *L )
{
	lua_pushinteger( L, luaL_checkmaterialvar( L, 1 )->VectorSize() );
	return 1;
}


static int IMaterialVar_GetTextureValue( lua_State *L )
{
	ITexture *pTex = luaL_checkmaterialvar( L, 1 )->GetTextureValue();
	if ( pTex )
		lua_pushtexture( L, pTex );
	else
		lua_pushnil( L );
	return 1;
}

static int IMaterialVar_SetTextureValue( lua_State *L )
{
	ITexture *pTex =
		lua_isnoneornil( L, 2 ) ? NULL : luaL_checktexture( L, 2 );
	luaL_checkmaterialvar( L, 1 )->SetTextureValue( pTex );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetMaterialValue( lua_State *L )
{
	IMaterial *pMat = luaL_checkmaterialvar( L, 1 )->GetMaterialValue();
	if ( pMat )
		lua_pushmaterial( L, pMat );
	else
		lua_pushnil( L );
	return 1;
}

static int IMaterialVar_SetMaterialValue( lua_State *L )
{
	IMaterial *pMat =
		lua_isnoneornil( L, 2 ) ? NULL : luaL_checkmaterial( L, 2 );
	luaL_checkmaterialvar( L, 1 )->SetMaterialValue( pMat );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_GetMatrixValue( lua_State *L )
{
	const VMatrix &m = luaL_checkmaterialvar( L, 1 )->GetMatrixValue();
	lua_pushvmatrix( L, m );
	return 1;
}

static int IMaterialVar_SetMatrixValue( lua_State *L )
{
	VMatrix m = luaL_checkvmatrix( L, 2 );
	luaL_checkmaterialvar( L, 1 )->SetMatrixValue( m );
	lua_settop( L, 1 );
	return 1;
}


static int IMaterialVar_tostring( lua_State *L )
{
	IMaterialVar *pVar = luaL_checkmaterialvar( L, 1 );
	lua_pushfstring( L, "IMaterialVar: %s (type %d)",
					 pVar->GetName(), (int)pVar->GetType() );
	return 1;
}

static int IMaterialVar_eq( lua_State *L )
{
	IMaterialVar **a = (IMaterialVar **)luaL_checkudata( L, 1, LUA_MATERIALVARLIBNAME );
	IMaterialVar **b = (IMaterialVar **)luaL_checkudata( L, 2, LUA_MATERIALVARLIBNAME );
	lua_pushboolean( L, a && b && *a == *b );
	return 1;
}


static const luaL_Reg IMaterialVar_methods[] = {
	{ "GetName",          IMaterialVar_GetName          },
	{ "GetType",          IMaterialVar_GetType          },
	{ "IsDefined",        IMaterialVar_IsDefined        },
	{ "SetUndefined",     IMaterialVar_SetUndefined     },

	{ "GetFloatValue",    IMaterialVar_GetFloatValue    },
	{ "SetFloatValue",    IMaterialVar_SetFloatValue    },

	{ "GetIntValue",      IMaterialVar_GetIntValue      },
	{ "SetIntValue",      IMaterialVar_SetIntValue      },

	{ "GetStringValue",   IMaterialVar_GetStringValue   },
	{ "SetStringValue",   IMaterialVar_SetStringValue   },

	{ "GetVecValue",      IMaterialVar_GetVecValue      },
	{ "SetVecValue",      IMaterialVar_SetVecValue      },
	{ "VectorSize",       IMaterialVar_VectorSize       },

	{ "GetTextureValue",  IMaterialVar_GetTextureValue  },
	{ "SetTextureValue",  IMaterialVar_SetTextureValue  },

	{ "GetMaterialValue", IMaterialVar_GetMaterialValue },
	{ "SetMaterialValue", IMaterialVar_SetMaterialValue },

	{ "GetMatrixValue",   IMaterialVar_GetMatrixValue   },
	{ "SetMatrixValue",   IMaterialVar_SetMatrixValue   },

	{ NULL, NULL }
};


LUALIB_API int luaopen_IMaterialVar( lua_State *L )
{
	luaL_newmetatable( L, LUA_MATERIALVARLIBNAME );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pushcfunction( L, IMaterialVar_tostring );
	lua_setfield( L, -2, "__tostring" );

	lua_pushcfunction( L, IMaterialVar_eq );
	lua_setfield( L, -2, "__eq" );

	luaL_register( L, NULL, IMaterialVar_methods );
	lua_pop( L, 1 );

	return 0;
}
