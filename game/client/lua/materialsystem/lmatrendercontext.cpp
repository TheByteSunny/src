//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "materialsystem/imaterialsystem.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "lmatrendercontext.h"
#include "limesh.h"
#include "materialsystem/limaterial.h"
#include "litexture.h"
#include "mathlib/lvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


/*
** access functions (stack -> C)
*/

LUALIB_API lua_MatRenderContextRef *luaL_checkmatrendercontextref( lua_State *L, int idx )
{
	lua_MatRenderContextRef *ref =
		(lua_MatRenderContextRef *)luaL_checkudata( L, idx, LUA_MATRENDERCONTEXTLIBNAME );
	if ( !ref || !ref->m_pContext )
		luaL_error( L, "IMatRenderContext is invalid" );
	return ref;
}

LUALIB_API IMatRenderContext *luaL_checkmatrendercontext( lua_State *L, int idx )
{
	return luaL_checkmatrendercontextref( L, idx )->m_pContext;
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushmatrendercontext( lua_State *L, IMatRenderContext *pCtx )
{
	if ( !pCtx )
	{
		lua_pushnil( L );
		return;
	}

	lua_MatRenderContextRef *ref = (lua_MatRenderContextRef *)
		lua_newuserdata( L, sizeof( lua_MatRenderContextRef ) );

	pCtx->AddRef();
	ref->m_pContext = pCtx;
	ref->m_bOwnsRef = true;

	luaL_getmetatable( L, LUA_MATRENDERCONTEXTLIBNAME );
	lua_setmetatable( L, -2 );
}



static int render_GetRenderContext( lua_State *L )
{
	IMatRenderContext *pCtx = materials->GetRenderContext();
	lua_pushmatrendercontext( L, pCtx );
	return 1;
}


static int IMatRenderContext_Bind( lua_State *L )
{
	IMatRenderContext *pCtx = luaL_checkmatrendercontext( L, 1 );
	IMaterial         *pMat = luaL_checkmaterial( L, 2 );

	pCtx->Bind( pMat, NULL );

	lua_settop( L, 1 );
	return 1;
}


static int IMatRenderContext_GetDynamicMesh( lua_State *L )
{
	IMatRenderContext *pCtx = luaL_checkmatrendercontext( L, 1 );

	bool       buffered = lua_isboolean( L, 2 ) ? lua_toboolean( L, 2 ) != 0 : true;
	IMaterial *pMat     = lua_isuserdata( L, 3 ) ? luaL_checkmaterial( L, 3 ) : NULL;
	IMesh     *pVtxOver = NULL; // todo

	IMesh *pMesh = pCtx->GetDynamicMesh( buffered, pVtxOver, NULL, pMat );
	lua_pushimesh( L, pMesh, false );
	return 1;
}



static int IMatRenderContext_MatrixMode( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->MatrixMode(
		(MaterialMatrixMode_t)luaL_checkint( L, 2 ) );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_PushMatrix( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->PushMatrix();
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_PopMatrix( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->PopMatrix();
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_LoadIdentity( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->LoadIdentity();
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_Translate( lua_State *L )
{
	IMatRenderContext *pCtx = luaL_checkmatrendercontext( L, 1 );
	if ( lua_gettop( L ) == 2 )
	{
		Vector v = luaL_checkvector( L, 2 );
		pCtx->Translate( v.x, v.y, v.z );
	}
	else
	{
		pCtx->Translate(
			(float)luaL_checknumber( L, 2 ),
			(float)luaL_checknumber( L, 3 ),
			(float)luaL_checknumber( L, 4 ) );
	}
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_Rotate( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->Rotate(
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ),
		(float)luaL_checknumber( L, 5 ) );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_Scale( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->Scale(
		(float)luaL_checknumber( L, 2 ),
		(float)luaL_checknumber( L, 3 ),
		(float)luaL_checknumber( L, 4 ) );
	lua_settop( L, 1 );
	return 1;
}


static int IMatRenderContext_PushRenderTargetAndViewport( lua_State *L )
{
	IMatRenderContext *pCtx = luaL_checkmatrendercontext( L, 1 );
	int top = lua_gettop( L );

	if ( top == 1 )
	{
		pCtx->PushRenderTargetAndViewport();
	}
	else if ( top == 2 )
	{
		ITexture *pRT = luaL_checktexture( L, 2 );
		pCtx->PushRenderTargetAndViewport( pRT );
	}
	else
	{
		// (rt, x, y, w, h)
		ITexture *pRT = luaL_checktexture( L, 2 );
		pCtx->PushRenderTargetAndViewport( pRT,
			luaL_checkint( L, 3 ),
			luaL_checkint( L, 4 ),
			luaL_checkint( L, 5 ),
			luaL_checkint( L, 6 ) );
	}
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_PopRenderTargetAndViewport( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->PopRenderTargetAndViewport();
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_SetRenderTarget( lua_State *L )
{
	IMatRenderContext *pCtx = luaL_checkmatrendercontext( L, 1 );
	ITexture *pRT = lua_isnoneornil( L, 2 ) ? NULL : luaL_checktexture( L, 2 );
	pCtx->SetRenderTarget( pRT );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_GetRenderTarget( lua_State *L )
{
	ITexture *pRT = luaL_checkmatrendercontext( L, 1 )->GetRenderTarget();
	if ( pRT )
		lua_pushtexture( L, pRT );
	else
		lua_pushnil( L );
	return 1;
}


static int IMatRenderContext_Viewport( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->Viewport(
		luaL_checkint( L, 2 ),
		luaL_checkint( L, 3 ),
		luaL_checkint( L, 4 ),
		luaL_checkint( L, 5 ) );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_GetViewport( lua_State *L )
{
	int x, y, w, h;
	luaL_checkmatrendercontext( L, 1 )->GetViewport( x, y, w, h );
	lua_pushinteger( L, x );
	lua_pushinteger( L, y );
	lua_pushinteger( L, w );
	lua_pushinteger( L, h );
	return 4;
}

static int IMatRenderContext_ClearColor3ub( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->ClearColor3ub(
		(unsigned char)luaL_checkint( L, 2 ),
		(unsigned char)luaL_checkint( L, 3 ),
		(unsigned char)luaL_checkint( L, 4 ) );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_ClearColor4ub( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->ClearColor4ub(
		(unsigned char)luaL_checkint( L, 2 ),
		(unsigned char)luaL_checkint( L, 3 ),
		(unsigned char)luaL_checkint( L, 4 ),
		(unsigned char)luaL_checkint( L, 5 ) );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_ClearBuffers( lua_State *L )
{
	bool color   = lua_toboolean( L, 2 ) != 0;
	bool depth   = lua_toboolean( L, 3 ) != 0;
	bool stencil = lua_isboolean( L, 4 ) ? lua_toboolean( L, 4 ) != 0 : false;
	luaL_checkmatrendercontext( L, 1 )->ClearBuffers( color, depth, stencil );
	lua_settop( L, 1 );
	return 1;
}


static int IMatRenderContext_GetRenderTargetDimensions( lua_State *L )
{
	int w, h;
	luaL_checkmatrendercontext( L, 1 )->GetRenderTargetDimensions( w, h );
	lua_pushinteger( L, w );
	lua_pushinteger( L, h );
	return 2;
}

static int IMatRenderContext_OverrideDepthEnable( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->OverrideDepthEnable(
		lua_toboolean( L, 2 ) != 0,
		lua_toboolean( L, 3 ) != 0 );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_OverrideAlphaWriteEnable( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->OverrideAlphaWriteEnable(
		lua_toboolean( L, 2 ) != 0,
		lua_toboolean( L, 3 ) != 0 );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_OverrideColorWriteEnable( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->OverrideColorWriteEnable(
		lua_toboolean( L, 2 ) != 0,
		lua_toboolean( L, 3 ) != 0 );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_SetStencilEnable( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->SetStencilEnable( lua_toboolean( L, 2 ) != 0 );
	lua_settop( L, 1 );
	return 1;
}

static int IMatRenderContext_CullMode( lua_State *L )
{
	luaL_checkmatrendercontext( L, 1 )->CullMode(
		(MaterialCullMode_t)luaL_checkint( L, 2 ) );
	lua_settop( L, 1 );
	return 1;
}


static int IMatRenderContext_gc( lua_State *L )
{
	lua_MatRenderContextRef *ref =
		(lua_MatRenderContextRef *)luaL_checkudata( L, 1, LUA_MATRENDERCONTEXTLIBNAME );
	if ( ref && ref->m_pContext && ref->m_bOwnsRef )
	{
		ref->m_pContext->Release();
	}
	if ( ref )
	{
		ref->m_pContext = NULL;
		ref->m_bOwnsRef = false;
	}
	return 0;
}

static int IMatRenderContext_tostring( lua_State *L )
{
	lua_MatRenderContextRef *ref =
		(lua_MatRenderContextRef *)luaL_checkudata( L, 1, LUA_MATRENDERCONTEXTLIBNAME );
	lua_pushfstring( L, "IMatRenderContext: %p", ref->m_pContext );
	return 1;
}


static const luaL_Reg IMatRenderContext_methods[] = {
	{ "Bind",                          IMatRenderContext_Bind                          },
	{ "GetDynamicMesh",                IMatRenderContext_GetDynamicMesh                },

	{ "MatrixMode",                    IMatRenderContext_MatrixMode                    },
	{ "PushMatrix",                    IMatRenderContext_PushMatrix                    },
	{ "PopMatrix",                     IMatRenderContext_PopMatrix                     },
	{ "LoadIdentity",                  IMatRenderContext_LoadIdentity                  },
	{ "Translate",                     IMatRenderContext_Translate                     },
	{ "Rotate",                        IMatRenderContext_Rotate                        },
	{ "Scale",                         IMatRenderContext_Scale                         },

	{ "PushRenderTargetAndViewport",   IMatRenderContext_PushRenderTargetAndViewport   },
	{ "PopRenderTargetAndViewport",    IMatRenderContext_PopRenderTargetAndViewport    },
	{ "SetRenderTarget",               IMatRenderContext_SetRenderTarget               },
	{ "GetRenderTarget",               IMatRenderContext_GetRenderTarget               },
	{ "GetRenderTargetDimensions",     IMatRenderContext_GetRenderTargetDimensions     },

	{ "Viewport",                      IMatRenderContext_Viewport                      },
	{ "GetViewport",                   IMatRenderContext_GetViewport                   },

	{ "ClearColor3ub",                 IMatRenderContext_ClearColor3ub                 },
	{ "ClearColor4ub",                 IMatRenderContext_ClearColor4ub                 },
	{ "ClearBuffers",                  IMatRenderContext_ClearBuffers                  },

	{ "OverrideDepthEnable",           IMatRenderContext_OverrideDepthEnable           },
	{ "OverrideAlphaWriteEnable",      IMatRenderContext_OverrideAlphaWriteEnable      },
	{ "OverrideColorWriteEnable",      IMatRenderContext_OverrideColorWriteEnable      },
	{ "SetStencilEnable",              IMatRenderContext_SetStencilEnable              },
	{ "CullMode",                      IMatRenderContext_CullMode                      },

	{ NULL, NULL }
};


static const luaL_Reg render_lib[] = {
	{ "GetRenderContext", render_GetRenderContext },
	{ NULL, NULL }
};


LUALIB_API int luaopen_IMatRenderContext( lua_State *L )
{
	luaL_newmetatable( L, LUA_MATRENDERCONTEXTLIBNAME );

	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );

	lua_pushcfunction( L, IMatRenderContext_tostring );
	lua_setfield( L, -2, "__tostring" );

	lua_pushcfunction( L, IMatRenderContext_gc );
	lua_setfield( L, -2, "__gc" );

	luaL_register( L, NULL, IMatRenderContext_methods );
	lua_pop( L, 1 );

	luaL_register( L, "render", render_lib );
	lua_pop( L, 1 );

	return 0;
}
