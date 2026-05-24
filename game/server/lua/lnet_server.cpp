//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnet_shared.h"
#include "luamanager.h"
#include "player.h"
#include "recipientfilter.h"
#include "usermessages.h"
#include "lbaseplayer_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


static int g_iLuaNetMsgIndex = -1;
static const char *LUA_NET_USERMSG_NAME = "LuaNetMessage";

void LuaNet_RegisterUserMessage()
{
	int iIndex = usermessages->LookupUserMessage( LUA_NET_USERMSG_NAME );
	if ( iIndex == -1 )
		usermessages->Register( LUA_NET_USERMSG_NAME, -1 );

	g_iLuaNetMsgIndex = iIndex;
}


static char     g_LuaNetWriteData[ 256 ];
static bf_write g_LuaNetWriteBuf;
static bool     g_bLuaNetMessageActive = false;
static int      g_iLuaNetCurrentID     = -1;


int net_Start( lua_State *L )
{
	if ( g_bLuaNetMessageActive )
		return luaL_error( L, "net.Start called while another message was already started" );

	const char *name = luaL_checkstring( L, 1 );
	int id = LuaNet_GetStringID( name );
	if ( id < 0 )
		return luaL_error( L, "net.Start: '%s' is not a registered network string "
							  "(call util.AddNetworkString first)", name );

	g_LuaNetWriteBuf.StartWriting( g_LuaNetWriteData, sizeof( g_LuaNetWriteData ) );
	// message id
	g_LuaNetWriteBuf.WriteUBitLong( id, LUA_NET_STRING_TABLE_BITS );

	g_pLuaNetWriteBuf      = &g_LuaNetWriteBuf;
	g_bLuaNetMessageActive = true;
	g_iLuaNetCurrentID     = id;
	return 0;
}

static void SendCurrentMessage( IRecipientFilter &filter )
{
	if ( !g_bLuaNetMessageActive )
		return;

	bf_write *pMsg = engine->UserMessageBegin( &filter, g_iLuaNetMsgIndex );
	if ( pMsg )
	{
		pMsg->WriteBits( g_LuaNetWriteData, g_LuaNetWriteBuf.GetNumBitsWritten() );
		engine->MessageEnd();
	}

	g_pLuaNetWriteBuf      = NULL;
	g_bLuaNetMessageActive = false;
	g_iLuaNetCurrentID     = -1;
}


int net_Send( lua_State *L )
{
	if ( !g_bLuaNetMessageActive )
		return luaL_error( L, "net.Send called without a matching net.Start" );

	CSingleUserRecipientFilter *pSingle = NULL;
	CRecipientFilter            multi;
	multi.MakeReliable();

	if ( lua_istable( L, 1 ) )
	{
		// table of players
		int n = lua_objlen( L, 1 );
		for ( int i = 1; i <= n; ++i )
		{
			lua_rawgeti( L, 1, i );
			CBasePlayer *pPly = luaL_checkplayer( L, -1 );
			if ( pPly )
				multi.AddRecipient( pPly );
			lua_pop( L, 1 );
		}
		SendCurrentMessage( multi );
	}
	else
	{
		CBasePlayer *pPly = luaL_checkplayer( L, 1 );
		CSingleUserRecipientFilter filter( pPly );
		filter.MakeReliable();
		SendCurrentMessage( filter );
	}
	return 0;
}


int net_Broadcast( lua_State *L )
{
	if ( !g_bLuaNetMessageActive )
		return luaL_error( L, "net.Broadcast called without a matching net.Start" );

	CReliableBroadcastRecipientFilter filter;
	SendCurrentMessage( filter );
	return 0;
}


int net_SendToServer( lua_State *L )
{
	return luaL_error( L, "net.SendToServer is client-only" );
}


void LuaNet_SyncStringsToClient( CBasePlayer *pPlayer )
{
	if ( !pPlayer )
		return;

	CSingleUserRecipientFilter filter( pPlayer );
	filter.MakeReliable();

	bf_write *pMsg = engine->UserMessageBegin( &filter, g_iLuaNetMsgIndex );
	if ( !pMsg )
		return;

	// marker id
	pMsg->WriteUBitLong( 0xFFFF & ((1 << LUA_NET_STRING_TABLE_BITS) - 1),
						 LUA_NET_STRING_TABLE_BITS );

	engine->MessageEnd();
}


CON_COMMAND( lua_net_c2s, "Lua net client-to-server (internal)" )
{

	CBasePlayer *pPly = UTIL_GetCommandClient();
	if ( !pPly )
		return;

	if ( args.ArgC() < 3 )
		return;

	static float s_LastCall[ MAX_PLAYERS + 1 ] = {};
	int idx = pPly->entindex();
	if ( gpGlobals->curtime - s_LastCall[ idx ] < 0.05f )  // 20/sec
		return;

	s_LastCall[ idx ] = gpGlobals->curtime;

	int bytes = atoi( args[ 1 ] );
	if ( bytes <= 0 || bytes > 255 )
	{
		Warning( "lua_net_c2s: bogus length %d from %s\n",
				 bytes, pPly->GetPlayerName() );
		return;
	}

	const char *hex = args[ 2 ];
	if ( (int)Q_strlen( hex ) != bytes * 2 )
	{
		Warning( "lua_net_c2s: length mismatch from %s\n", pPly->GetPlayerName() );
		return;
	}

	char buf[ 256 ];
	for ( int i = 0; i < bytes; ++i )
	{
		unsigned int b;
		if ( sscanf( hex + i * 2, "%02x", &b ) != 1 )
			return;
		buf[ i ] = (char)b;
	}

	bf_read msg( buf, bytes );
	int id = msg.ReadUBitLong( LUA_NET_STRING_TABLE_BITS );
	const char *name = LuaNet_GetStringName( id );
	if ( !name )
	{
		Warning( "lua_net_c2s: unknown string ID %d from %s\n",
				 id, pPly->GetPlayerName() );
		return;
	}

	int bitsLeft = msg.GetNumBitsLeft();

	g_pLuaNetReadBuf = &msg;
	LuaNet_DispatchReceive( L, name, bitsLeft, pPly );
	g_pLuaNetReadBuf = NULL;
}

