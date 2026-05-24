//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "materialsystem/itexture.h"
#include "luamanager.h"
#include "luasrclib.h"
#include "litexture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** access functions (stack -> C)
*/

LUA_API lua_ITexture *lua_totexture( lua_State *L, int idx )
{
	lua_ITexture **ppTex = (lua_ITexture **)lua_touserdata( L, idx );
	if ( ppTex == NULL )
		return NULL;
	return *ppTex;
}

LUALIB_API lua_ITexture *luaL_checktexture( lua_State *L, int narg )
{
	lua_ITexture **d = (lua_ITexture **)luaL_checkudata( L, narg, LUA_TEXTURELIBNAME );
	return *d;
}

LUALIB_API lua_ITexture *luaL_opttexture( lua_State *L, int narg, lua_ITexture *def )
{
	if ( lua_isnoneornil( L, narg ) )
		return def;
	return luaL_checktexture( L, narg );
}


/*
** push functions (C -> stack)
*/

LUA_API void lua_pushtexture( lua_State *L, lua_ITexture *pTexture )
{
	if ( pTexture == NULL )
		lua_pushnil( L );
	else
	{
		lua_ITexture **ppTex = (lua_ITexture **)lua_newuserdata( L, sizeof( pTexture ) );
		*ppTex = pTexture;
		luaL_getmetatable( L, LUA_TEXTURELIBNAME );
		lua_setmetatable( L, -2 );
	}
}

static int ITexture_GetName( lua_State *L )
{
	lua_pushstring( L, luaL_checktexture( L, 1 )->GetName() );
	return 1;
}

static int ITexture_GetActualWidth( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetActualWidth() );
	return 1;
}

static int ITexture_GetActualHeight( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetActualHeight() );
	return 1;
}

static int ITexture_GetMappingWidth( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetMappingWidth() );
	return 1;
}

static int ITexture_GetMappingHeight( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetMappingHeight() );
	return 1;
}

static int ITexture_GetNumAnimationFrames( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetNumAnimationFrames() );
	return 1;
}

static int ITexture_IsTranslucent( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsTranslucent() );
	return 1;
}

static int ITexture_IsMipmapped( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsMipmapped() );
	return 1;
}

static int ITexture_IsNormalMap( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsNormalMap() );
	return 1;
}

static int ITexture_IsCubeMap( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsCubeMap() );
	return 1;
}

static int ITexture_IsRenderTarget( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsRenderTarget() );
	return 1;
}

static int ITexture_IsProcedural( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsProcedural() );
	return 1;
}

static int ITexture_IsError( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsError() );
	return 1;
}

static int ITexture_IsVolumeTexture( lua_State *L )
{
	lua_pushboolean( L, luaL_checktexture( L, 1 )->IsVolumeTexture() );
	return 1;
}

static int ITexture_GetImageFormat( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetImageFormat() );
	return 1;
}

// signature: GetLowResColorSample( float s, float t, float *color )
//   color is a 4-float array, RGBA
static int ITexture_GetLowResColorSample( lua_State *L )
{
	float color[4] = { 0, 0, 0, 0 };
	luaL_checktexture( L, 1 )->GetLowResColorSample( (float)luaL_checknumber( L, 2 ), (float)luaL_checknumber( L, 3 ), color );
	lua_pushnumber( L, color[0] );
	lua_pushnumber( L, color[1] );
	lua_pushnumber( L, color[2] );
	lua_pushnumber( L, color[3] );
	return 4;
}

static int ITexture_IncrementReferenceCount( lua_State *L )
{
	luaL_checktexture( L, 1 )->IncrementReferenceCount();
	return 0;
}

static int ITexture_DecrementReferenceCount( lua_State *L )
{
	luaL_checktexture( L, 1 )->DecrementReferenceCount();
	return 0;
}

static int ITexture_DeleteIfUnreferenced( lua_State *L )
{
	luaL_checktexture( L, 1 )->DeleteIfUnreferenced();
	return 0;
}

static int ITexture_Download( lua_State *L )
{
	// optional Rect_t arg ignored
	luaL_checktexture( L, 1 )->Download();
	return 0;
}

static int ITexture_SwapContents( lua_State *L )
{
	luaL_checktexture( L, 1 )->SwapContents( luaL_checktexture( L, 2 ) );
	return 0;
}

static int ITexture_GetApproximateVidMemBytes( lua_State *L )
{
	lua_pushinteger( L, luaL_checktexture( L, 1 )->GetApproximateVidMemBytes() );
	return 1;
}

static int ITexture___eq( lua_State *L )
{
	lua_pushboolean( L, lua_totexture( L, 1 ) == lua_totexture( L, 2 ) );
	return 1;
}

static int ITexture___tostring( lua_State *L )
{
	lua_ITexture *pTex = lua_totexture( L, 1 );
	if ( pTex == NULL )
		lua_pushstring( L, "NULL_TEXTURE" );
	else
		lua_pushfstring( L, "ITexture: %s", pTex->GetName() );
	return 1;
}

static const luaL_Reg ITexturemeta[] = {
  {"GetName",                    ITexture_GetName},
  {"GetActualWidth",             ITexture_GetActualWidth},
  {"GetActualHeight",            ITexture_GetActualHeight},
  {"GetMappingWidth",            ITexture_GetMappingWidth},
  {"GetMappingHeight",           ITexture_GetMappingHeight},
  {"GetNumAnimationFrames",      ITexture_GetNumAnimationFrames},
  {"IsTranslucent",              ITexture_IsTranslucent},
  {"IsMipmapped",                ITexture_IsMipmapped},
  {"IsNormalMap",                ITexture_IsNormalMap},
  {"IsCubeMap",                  ITexture_IsCubeMap},
  {"IsRenderTarget",             ITexture_IsRenderTarget},
  {"IsProcedural",               ITexture_IsProcedural},
  {"IsError",                    ITexture_IsError},
  {"IsVolumeTexture",            ITexture_IsVolumeTexture},
  {"GetImageFormat",             ITexture_GetImageFormat},
  {"GetLowResColorSample",       ITexture_GetLowResColorSample},
  {"IncrementReferenceCount",    ITexture_IncrementReferenceCount},
  {"DecrementReferenceCount",    ITexture_DecrementReferenceCount},
  {"DeleteIfUnreferenced",       ITexture_DeleteIfUnreferenced},
  {"Download",                   ITexture_Download},
  {"SwapContents",               ITexture_SwapContents},
  {"GetApproximateVidMemBytes",  ITexture_GetApproximateVidMemBytes},
  {"__eq",                       ITexture___eq},
  {"__tostring",                 ITexture___tostring},
  {NULL, NULL}
};


/*
** Open ITexture object
*/
LUALIB_API int luaopen_ITexture( lua_State *L )
{
	luaL_newmetatable( L, LUA_TEXTURELIBNAME );
	luaL_register( L, NULL, ITexturemeta );
	lua_pushvalue( L, -1 );
	lua_setfield( L, -2, "__index" );
	lua_pushstring( L, "texture" );
	lua_setfield( L, -2, "__type" );

	lua_pushtexture( L, NULL );
	lua_setglobal( L, "NULL_TEXTURE" );
	return 1;
}
