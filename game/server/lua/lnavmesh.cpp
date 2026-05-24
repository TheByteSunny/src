//========== Copyright (C) 2026, The Aridity Team, All rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "lnavmesh.h"
#include "nav_area.h"
#include "nav_mesh.h"
#include "mathlib/lvector.h"
#include "luasrclib.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

/*
** push functions (C -> stack)
*/
LUA_API void lua_pushnavarea(lua_State *L, CNavArea *area) {
	if (!area) {
		lua_pushnil(L);
		return;
	}
	CNavArea **ppArea = (CNavArea **)lua_newuserdata(L, sizeof(CNavArea *));
	*ppArea = area;
	luaL_getmetatable(L, LUA_NAVAREALIBNAME);
	lua_setmetatable(L, -2);
}

/*
** access functions (stack -> C)
*/
LUA_API CNavArea *lua_tonavarea(lua_State *L, int idx) {
	CNavArea **ppArea = (CNavArea **)lua_touserdata(L, idx);
	return ppArea ? *ppArea : nullptr;
}

LUA_API CNavArea *luaL_checknavarea(lua_State *L, int narg) {
	CNavArea **ppArea = (CNavArea **)luaL_checkudata(L, narg, LUA_NAVAREALIBNAME);
	if (!ppArea || !*ppArea)
		luaL_argerror(L, narg, "CNavArea expected");
	return *ppArea;
}

/*
** CNavArea methods
*/
static int CNavArea_GetID(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushinteger(L, (lua_Integer)area->GetID());
	return 1;
}

static int CNavArea_GetCenter(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	const Vector &center = area->GetCenter();
	lua_pushvector(L, center);
	return 1;
}

static int CNavArea_GetSizeX(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushnumber(L, area->GetSizeX());
	return 1;
}

static int CNavArea_GetSizeY(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushnumber(L, area->GetSizeY());
	return 1;
}

static int CNavArea_GetCorner(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int corner = (int)luaL_checkint(L, 2);
	if (corner < 0 || corner >= NUM_CORNERS)
		luaL_argerror(L, 2, "corner index out of range (0..3)");
	Vector v = area->GetCorner((NavCornerType)corner);
	lua_pushvector(L, v);
	return 1;
}

static int CNavArea_SetCorner(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int corner = (int)luaL_checkint(L, 2);
	if (corner < 0 || corner >= NUM_CORNERS)
		luaL_argerror(L, 2, "corner index out of range (0..3)");
	Vector v = luaL_checkvector(L, 3);
	area->SetCorner((NavCornerType)corner, v);
	return 0;
}

static int CNavArea_GetRandomPoint(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector v = area->GetRandomPoint();
	lua_pushvector(L, v);
	return 1;
}

static int CNavArea_IsBlocked(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_optint(L, 2, TEAM_ANY);
	bool val = area->IsBlocked(teamID, true);
	lua_pushboolean(L, val);
	return 1;
}

static int CNavArea_SetBlocked(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	bool blocked = lua_toboolean(L, 2) != 0;
	int teamID = (int)luaL_optint(L, 3, TEAM_ANY);

	if (blocked)
		area->MarkAsBlocked(teamID, NULL, true);
	else
		area->UnblockArea(teamID);
	return 0;
}

static int CNavArea_Contains(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector pos = luaL_checkvector(L, 2);
	lua_pushboolean(L, area->Contains(pos));
	return 1;
}

static int CNavArea_GetClosestPointOnArea(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector pos = luaL_checkvector(L, 2);
	Vector close;
	area->GetClosestPointOnArea(&pos, &close);
	lua_pushvector(L, close);
	return 1;
}

static int CNavArea_GetDistanceSquaredToPoint(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector pos = luaL_checkvector(L, 2);
	float dsq = area->GetDistanceSquaredToPoint(pos);
	lua_pushnumber(L, dsq);
	return 1;
}

static int CNavArea_GetLightIntensityAtPos(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector pos = luaL_checkvector(L, 2);
	float intensity = area->GetLightIntensity(pos);
	lua_pushnumber(L, intensity);
	return 1;
}

static int CNavArea_GetLightIntensity(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushnumber(L, area->GetLightIntensity());
	return 1;
}

static int CNavArea_GetAdjacentCount(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int dir = (int)luaL_optint(L, 2, 0);
	lua_pushinteger(L, area->GetAdjacentCount((NavDirType)dir));
	return 1;
}

static int CNavArea_GetAdjacentArea(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int dir = (int)luaL_optint(L, 2, 0);
	int idx = (int)luaL_checkint(L, 3);
	CNavArea *adj = area->GetAdjacentArea((NavDirType)dir, idx);
	lua_pushnavarea(L, adj);
	return 1;
}

static int CNavArea_GetRandomAdjacentArea(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int dir = (int)luaL_optint(L, 2, 0);
	CNavArea *adj = area->GetRandomAdjacentArea((NavDirType)dir);
	lua_pushnavarea(L, adj);
	return 1;
}

static int CNavArea_GetAllAdjacentAreas(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_newtable(L);
	int i = 1;
	for (int dir = 0; dir < NUM_DIRECTIONS; ++dir)
	{
		int count = area->GetAdjacentCount((NavDirType)dir);
		for (int idx = 0; idx < count; ++idx)
		{
			CNavArea *adj = area->GetAdjacentArea((NavDirType)dir, idx);
			if (adj)
			{
				lua_pushinteger(L, i++);
				lua_pushnavarea(L, adj);
				lua_settable(L, -3);
			}
		}
	}
	return 1;
}

static int CNavArea_GetPlayerCount(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_optint(L, 2, 0);
	lua_pushinteger(L, area->GetPlayerCount(teamID));
	return 1;
}

static int CNavArea_GetHidingSpots(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	const HidingSpotVector *spots = area->GetHidingSpots();
	lua_newtable(L);
	int i = 1;
	for (int s = 0; s < spots->Count(); ++s)
	{
		const HidingSpot &hs = *(*spots)[s];
		lua_pushinteger(L, i++);
		lua_pushvector(L, hs.GetPosition());
		lua_settable(L, -3);
	}
	return 1;
}

static int CNavArea_GetAttributes(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushinteger(L, area->GetAttributes());
	return 1;
}

static int CNavArea_SetAttributes(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int bits = (int)luaL_checkint(L, 2);
	area->SetAttributes(bits);
	return 0;
}

static int CNavArea_HasAttributes(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int bits = (int)luaL_checkint(L, 2);
	lua_pushboolean(L, area->HasAttributes(bits));
	return 1;
}

static int CNavArea_RemoveAttributes(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int bits = (int)luaL_checkint(L, 2);
	area->RemoveAttributes(bits);
	return 0;
}

static int CNavArea_GetPlace(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushinteger(L, (lua_Integer)area->GetPlace());
	return 1;
}

static int CNavArea_SetPlace(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Place place = (Place)luaL_checkint(L, 2);
	area->SetPlace(place);
	return 0;
}

static int CNavArea_GetZ(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	float x = (float)luaL_checknumber(L, 2);
	float y = (float)luaL_checknumber(L, 3);
	lua_pushnumber(L, area->GetZ(x, y));
	return 1;
}

static int CNavArea_IsOverlapping(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	CNavArea *other = lua_tonavarea(L, 2);
	if (other)
	{
		lua_pushboolean(L, area->IsOverlapping(other));
	}
	else
	{
		Vector pos = luaL_checkvector(L, 2);
		float tolerance = (float)luaL_optnumber(L, 3, 0.0f);
		lua_pushboolean(L, area->IsOverlapping(pos, tolerance));
	}
	return 1;
}

static int CNavArea_IsCoplanar(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	CNavArea *other = luaL_checknavarea(L, 2);
	lua_pushboolean(L, area->IsCoplanar(other));
	return 1;
}

static int CNavArea_IsDegenerate(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsDegenerate());
	return 1;
}

static int CNavArea_IsFlat(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsFlat());
	return 1;
}

static int CNavArea_IsRoughlySquare(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsRoughlySquare());
	return 1;
}

static int CNavArea_IsEdge(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int dir = (int)luaL_checkint(L, 2);
	if (dir < 0 || dir >= NUM_DIRECTIONS)
		luaL_argerror(L, 2, "direction out of range (0..3)");
	lua_pushboolean(L, area->IsEdge((NavDirType)dir));
	return 1;
}

static int CNavArea_GetExtent(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Extent ext;
	area->GetExtent(&ext);
	lua_pushvector(L, ext.lo);
	lua_pushvector(L, ext.hi);
	return 2;
}

static int CNavArea_IsConnected(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	CNavArea *other = luaL_checknavarea(L, 2);
	int dir = (int)luaL_optint(L, 3, NUM_DIRECTIONS);
	if (dir == NUM_DIRECTIONS)
	{
		// check all dirs
		bool connected = false;
		for (int d = 0; d < NUM_DIRECTIONS; ++d)
		{
			if (area->IsConnected(other, (NavDirType)d))
			{
				connected = true;
				break;
			}
		}
		lua_pushboolean(L, connected);
	}
	else
	{
		lua_pushboolean(L, area->IsConnected(other, (NavDirType)dir));
	}
	return 1;
}

static int CNavArea_IncreaseDanger(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_checkint(L, 2);
	float amount = (float)luaL_checknumber(L, 3);
	area->IncreaseDanger(teamID, amount);
	return 0;
}

static int CNavArea_GetDanger(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_checkint(L, 2);
	lua_pushnumber(L, area->GetDanger(teamID));
	return 1;
}

static int CNavArea_IsVisible(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	Vector eye = luaL_checkvector(L, 2);
	lua_pushboolean(L, area->IsVisible(eye));
	return 1;
}

static int CNavArea_IsPotentiallyVisible(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	CNavArea *other = luaL_checknavarea(L, 2);
	lua_pushboolean(L, area->IsPotentiallyVisible(other));
	return 1;
}

static int CNavArea_IsCompletelyVisible(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	CNavArea *other = luaL_checknavarea(L, 2);
	lua_pushboolean(L, area->IsCompletelyVisible(other));
	return 1;
}

static int CNavArea_IsDamaging(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsDamaging());
	return 1;
}

static int CNavArea_MarkAsDamaging(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	float duration = (float)luaL_checknumber(L, 2);
	area->MarkAsDamaging(duration);
	return 0;
}

static int CNavArea_IsUnderwater(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsUnderwater());
	return 1;
}

static int CNavArea_IsBattlefront(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushboolean(L, area->IsBattlefront());
	return 1;
}

static int CNavArea_HasAvoidanceObstacle(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	float maxHeight = (float)luaL_optnumber(L, 2, StepHeight);
	lua_pushboolean(L, area->HasAvoidanceObstacle(maxHeight));
	return 1;
}

static int CNavArea_GetAvoidanceObstacleHeight(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushnumber(L, area->GetAvoidanceObstacleHeight());
	return 1;
}

static int CNavArea_GetEarliestOccupyTime(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_checkint(L, 2);
	lua_pushnumber(L, area->GetEarliestOccupyTime(teamID));
	return 1;
}

static int CNavArea_GetClearedTimestamp(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_checkint(L, 2);
	lua_pushnumber(L, area->GetClearedTimestamp(teamID));
	return 1;
}

static int CNavArea_SetClearedTimestamp(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	int teamID = (int)luaL_checkint(L, 2);
	area->SetClearedTimestamp(teamID);
	return 0;
}

static int CNavArea___tostring(lua_State *L)
{
	CNavArea *area = luaL_checknavarea(L, 1);
	lua_pushfstring(L, "CNavArea[%d] (%p)", area->GetID(), area);
	return 1;
}

static const luaL_Reg CNavArea_meta[] = {
	{"GetID",                       CNavArea_GetID},
	{"GetCenter",                   CNavArea_GetCenter},
	{"GetSizeX",                    CNavArea_GetSizeX},
	{"GetSizeY",                    CNavArea_GetSizeY},
	{"GetExtent",                   CNavArea_GetExtent},
	{"GetCorner",                   CNavArea_GetCorner},
	{"SetCorner",                   CNavArea_SetCorner},
	{"GetRandomPoint",              CNavArea_GetRandomPoint},
	{"GetZ",                        CNavArea_GetZ},
	{"IsBlocked",                   CNavArea_IsBlocked},
	{"SetBlocked",                  CNavArea_SetBlocked},
	{"Contains",                    CNavArea_Contains},
	{"IsOverlapping",               CNavArea_IsOverlapping},
	{"IsCoplanar",                  CNavArea_IsCoplanar},
	{"IsDegenerate",                CNavArea_IsDegenerate},
	{"IsFlat",                      CNavArea_IsFlat},
	{"IsRoughlySquare",             CNavArea_IsRoughlySquare},
	{"IsEdge",                      CNavArea_IsEdge},
	{"GetClosestPointOnArea",       CNavArea_GetClosestPointOnArea},
	{"GetDistanceSquaredToPoint",   CNavArea_GetDistanceSquaredToPoint},
	{"IsConnected",                 CNavArea_IsConnected},
	{"GetAdjacentCount",            CNavArea_GetAdjacentCount},
	{"GetAdjacentArea",             CNavArea_GetAdjacentArea},
	{"GetRandomAdjacentArea",       CNavArea_GetRandomAdjacentArea},
	{"GetAllAdjacentAreas",         CNavArea_GetAllAdjacentAreas},
	{"GetAttributes",               CNavArea_GetAttributes},
	{"SetAttributes",               CNavArea_SetAttributes},
	{"HasAttributes",               CNavArea_HasAttributes},
	{"RemoveAttributes",            CNavArea_RemoveAttributes},
	{"GetPlace",                    CNavArea_GetPlace},
	{"SetPlace",                    CNavArea_SetPlace},
	{"GetPlayerCount",              CNavArea_GetPlayerCount},
	{"GetLightIntensityAtPos",      CNavArea_GetLightIntensityAtPos},
	{"GetLightIntensity",           CNavArea_GetLightIntensity},
	{"GetHidingSpots",              CNavArea_GetHidingSpots},
	{"IncreaseDanger",              CNavArea_IncreaseDanger},
	{"GetDanger",                   CNavArea_GetDanger},
	{"IsVisible",                   CNavArea_IsVisible},
	{"IsPotentiallyVisible",        CNavArea_IsPotentiallyVisible},
	{"IsCompletelyVisible",         CNavArea_IsCompletelyVisible},
	{"IsDamaging",                  CNavArea_IsDamaging},
	{"MarkAsDamaging",              CNavArea_MarkAsDamaging},
	{"IsUnderwater",                CNavArea_IsUnderwater},
	{"IsBattlefront",               CNavArea_IsBattlefront},
	{"HasAvoidanceObstacle",        CNavArea_HasAvoidanceObstacle},
	{"GetAvoidanceObstacleHeight",  CNavArea_GetAvoidanceObstacleHeight},
	{"GetEarliestOccupyTime",       CNavArea_GetEarliestOccupyTime},
	{"GetClearedTimestamp",         CNavArea_GetClearedTimestamp},
	{"SetClearedTimestamp",         CNavArea_SetClearedTimestamp},
	{"GetAdjacentCount",            CNavArea_GetAdjacentCount},
	{"__tostring",                  CNavArea___tostring},
	{NULL, NULL}
};

LUALIB_API int luaopen_CNavArea(lua_State *L) {
	luaL_newmetatable(L, LUA_NAVAREALIBNAME);
	luaL_register(L, NULL, CNavArea_meta);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_pushstring(L, "navarea");
	lua_setfield(L, -2, "__type");
	lua_pop(L, 1);
	return 1;
}

/*
** Global navmesh library
*/
static int navmesh_GetAllNavAreas(lua_State *L) {
	lua_newtable(L);
	int i = 1;
	FOR_EACH_VEC(TheNavAreas, it) {
		CNavArea *area = TheNavAreas[it];
		if (area) {
			lua_pushinteger(L, i++);
			lua_pushnavarea(L, area);
			lua_settable(L, -3);
		}
	}
	return 1;
}

static int navmesh_GetNearestNavArea(lua_State *L) {
	Vector pos = luaL_checkvector(L, 1);
	float maxDist = (float)luaL_optnumber(L, 2, 10000.0f);
	bool checkLOS = lua_toboolean(L, 3) != 0;
	bool checkGround = lua_isnoneornil(L, 4) ? true : lua_toboolean(L, 4) != 0;

	CNavArea *area = TheNavMesh->GetNearestNavArea(pos, false, maxDist, checkLOS, checkGround);
	lua_pushnavarea(L, area);
	return 1;
}

static int navmesh_GetNavArea(lua_State *L) {
	Vector pos = luaL_checkvector(L, 1);
	float beneathLimit = (float)luaL_optnumber(L, 2, 120.0f);
	CNavArea *area = TheNavMesh->GetNavArea(pos, beneathLimit);
	lua_pushnavarea(L, area);
	return 1;
}

static int navmesh_GetNavAreaByID(lua_State *L) {
	unsigned int id = (unsigned int)luaL_checkint(L, 1);
	CNavArea *area = TheNavMesh->GetNavAreaByID(id);
	lua_pushnavarea(L, area);
	return 1;
}

static int navmesh_GetNavAreaCount(lua_State *L) {
	lua_pushinteger(L, (lua_Integer)TheNavMesh->GetNavAreaCount());
	return 1;
}

static int navmesh_IsLoaded(lua_State *L) {
	lua_pushboolean(L, TheNavMesh->IsLoaded());
	return 1;
}

static int navmesh_IsAnalyzed(lua_State *L) {
	lua_pushboolean(L, TheNavMesh->IsAnalyzed());
	return 1;
}

static int navmesh_GetGroundHeight(lua_State *L) {
	Vector pos = luaL_checkvector(L, 1);
	float height = 0.0f;
	bool ok = TheNavMesh->GetGroundHeight(pos, &height);
	lua_pushnumber(L, height);
	lua_pushboolean(L, ok);
	return 2;
}

static int navmesh_IncreaseDangerNearby(lua_State *L) {
	int teamID = (int)luaL_checkint(L, 1);
	float amount = (float)luaL_checknumber(L, 2);
	CNavArea *area = luaL_checknavarea(L, 3);
	Vector pos = luaL_checkvector(L, 4);
	float maxRadius = (float)luaL_checknumber(L, 5);
	float dangerLimit = (float)luaL_optnumber(L, 6, -1.0f);
	TheNavMesh->IncreaseDangerNearby(teamID, amount, area, pos, maxRadius, dangerLimit);
	return 0;
}

static const luaL_Reg navmesh_funcs[] = {
	{"GetAllNavAreas",        navmesh_GetAllNavAreas},
	{"GetNearestNavArea",     navmesh_GetNearestNavArea},
	{"GetNavArea",            navmesh_GetNavArea},
	{"GetNavAreaByID",        navmesh_GetNavAreaByID},
	{"GetNavAreaCount",       navmesh_GetNavAreaCount},
	{"IsLoaded",              navmesh_IsLoaded},
	{"IsAnalyzed",            navmesh_IsAnalyzed},
	{"GetGroundHeight",       navmesh_GetGroundHeight},
	{"IncreaseDangerNearby",  navmesh_IncreaseDangerNearby},
	{NULL, NULL}
};

LUALIB_API int luaopen_navmesh(lua_State *L) {
	luaL_register(L, LUA_NAVMESHLIBNAME, navmesh_funcs);
	return 1;
}
