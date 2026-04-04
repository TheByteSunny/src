// C++ Bit Extension made by YourLocalSunny \\

#define LUA_LIB
#include "lua.h"
#include "lauxlib.h"

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

typedef int32_t SBits;
typedef uint32_t UBits;

// all this using bit
static UBits barg(lua_State *L, int idx)
{
    return (UBits)luaL_checkinteger(L, idx);
}

#define BRET(b) lua_pushinteger(L, (lua_Integer)(SBits)(b)); return 1;

// here we have new funcs
static int bit_test(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits n = barg(L, 2) & 31;
    lua_pushboolean(L, (b & (1u << n)) != 0);
    return 1;
}

/* bit.set(x, n) */
static int bit_set(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits n = barg(L, 2) & 31;
    BRET(b | (1u << n))
}

/* bit.clear(x, n) */
static int bit_clear(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits n = barg(L, 2) & 31;
    BRET(b & ~(1u << n))
}

/* bit.toggle(x, n) */
static int bit_toggle(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits n = barg(L, 2) & 31;
    BRET(b ^ (1u << n))
}

/* bit.mask(n) -> (1<<n)-1 */
static int bit_mask(lua_State *L)
{
    UBits n = barg(L, 1);
    if (n >= 32) BRET(0xFFFFFFFFu)
    BRET((1u << n) - 1)
}

/* bit.extract(x, start, len) */
static int bit_extract(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits start = barg(L, 2) & 31;
    UBits len = barg(L, 3) & 31;

    if (len == 0) BRET(0)
    if (len >= 32) BRET(b >> start)

    BRET((b >> start) & ((1u << len) - 1))
}

/* bit.replace(x, v, start, len) */
static int bit_replace(lua_State *L)
{
    UBits b = barg(L, 1);
    UBits v = barg(L, 2);
    UBits start = barg(L, 3) & 31;
    UBits len = barg(L, 4) & 31;

    UBits mask = ((1u << len) - 1u) << start;
    b = (b & ~mask) | ((v << start) & mask);

    BRET(b)
}

/* popcount */
static int bit_popcount(lua_State *L)
{
    UBits b = barg(L, 1);

#if defined(__GNUC__)
    BRET(__builtin_popcount(b))
#else
    /* fallback */
    int count = 0;
    while (b) { b &= (b - 1); count++; }
    lua_pushinteger(L, count);
    return 1;
#endif
}

static const struct luaL_Reg bit_ext_funcs[] = {
    { "test", bit_test },
    { "set", bit_set },
    { "clear", bit_clear },
    { "toggle", bit_toggle },
    { "mask", bit_mask },
    { "extract", bit_extract },
    { "replace", bit_replace },
    { "popcount", bit_popcount },
    { NULL, NULL }
};

LUALIB_API int luaopen_bit_ext(lua_State *L)
{
    lua_getglobal(L, "bit");

    if (!lua_istable(L, -1))
    {
        luaL_error(L, "bit library not found");
        return 0;
    }
  
    luaL_setfuncs(L, bit_ext_funcs, 0);

    return 1;
}
