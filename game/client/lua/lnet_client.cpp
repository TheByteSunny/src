//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnet_shared.h"
#include "luamanager.h"
#include "usermessages.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static const char *LUA_NET_USERMSG_NAME = "LuaNetMessage";


static char     g_LuaNetWriteData[ 256 ];
static bf_write g_LuaNetWriteBuf;
static bool     g_bLuaNetMessageActive = false;
static int      g_iLuaNetCurrentID     = -1;


static void LuaNet_HandleSync( bf_read &msg )
{
	int count = msg.ReadUBitLong( 16 );
	for ( int i = 0; i < count; ++i )
	{
		int id = msg.ReadUBitLong( LUA_NET_STRING_TABLE_BITS );
		char name[ 256 ];
		msg.ReadString( name, sizeof( name ) );

		// insert at correct id
		extern void LuaNet_SetString( int id, const char *name );
		LuaNet_SetString( id, name );
	}
}

static void __MsgFunc_LuaNetMessage( bf_read &msg )
{
	int id = msg.ReadUBitLong( LUA_NET_STRING_TABLE_BITS );

	int syncID = LuaNet_GetStringID( "__lua_net_sync__" );

	if ( syncID < 0 )
	{
		LuaNet_HandleSync( msg );
		return;
	}

	if ( id == syncID )
	{
		LuaNet_HandleSync( msg );
		return;
	}

	const char *name = LuaNet_GetStringName( id );
	if ( !name )
	{
		Warning( "LuaNet: received message with unknown ID %d\n", id );
		return;
	}

	int bitsLeft = msg.GetNumBitsLeft();

	g_pLuaNetReadBuf = &msg;
	LuaNet_DispatchReceive( L, name, bitsLeft, NULL );
	g_pLuaNetReadBuf = NULL;
}

void LuaNet_ClientInit()
{
	usermessages->HookMessage( LUA_NET_USERMSG_NAME, __MsgFunc_LuaNetMessage );
}


int net_Start( lua_State *L )
{
	if ( g_bLuaNetMessageActive )
		return luaL_error( L, "net.Start called while another message was already started" );

	const char *name = luaL_checkstring( L, 1 );
	int id = LuaNet_GetStringID( name );
	if ( id < 0 )
		return luaL_error( L, "net.Start: '%s' is not a registered network string", name );

	g_LuaNetWriteBuf.StartWriting( g_LuaNetWriteData, sizeof( g_LuaNetWriteData ) );
	g_LuaNetWriteBuf.WriteUBitLong( id, LUA_NET_STRING_TABLE_BITS );

	g_pLuaNetWriteBuf      = &g_LuaNetWriteBuf;
	g_bLuaNetMessageActive = true;
	g_iLuaNetCurrentID     = id;
	return 0;
}

int net_SendToServer( lua_State *L )
{
	if ( !g_bLuaNetMessageActive )
		return luaL_error( L, "net.SendToServer called without a matching net.Start" );


	// чооо эта ваще законна!?>1/?!1.;3w.le,qaemaijdiosa
	int bytes = g_LuaNetWriteBuf.GetNumBytesWritten();
	char hex[ 256 * 2 + 64 ];
	int  o = 0;
	o += Q_snprintf( hex + o, sizeof( hex ) - o, "lua_net_c2s %d ", bytes );
	for ( int i = 0; i < bytes; ++i )
	{
		o += Q_snprintf( hex + o, sizeof( hex ) - o, "%02x",
						 (unsigned char)g_LuaNetWriteData[ i ] );
	}

	engine->ClientCmd_Unrestricted( hex );

	g_pLuaNetWriteBuf      = NULL;
	g_bLuaNetMessageActive = false;
	g_iLuaNetCurrentID     = -1;
	return 0;
}

int net_Send     ( lua_State *L ) { return luaL_error( L, "net.Send is server-only" ); }
int net_Broadcast( lua_State *L ) { return luaL_error( L, "net.Broadcast is server-only" ); }
