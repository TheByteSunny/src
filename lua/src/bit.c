/*
** Lua BitOp -- a bit operations library for Lua 5.1/5.2.
** http://bitop.luajit.org/
**
** Copyright (C) 2008-2012 Mike Pall. All rights reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
*/

#define LUA_BITOP_VERSION	"1.0.2"

#define LUA_LIB
#include "lua.h"
#include "lauxlib.h"

#ifdef _MSC_VER
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef signed __int64 SBits64;
typedef unsigned __int64 UBits64;

typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
typedef int64_t SBits64;
typedef uint64_t UBits64;
#endif

typedef int32_t SBits;
typedef uint32_t UBits;

typedef union {
  lua_Number n;
#ifdef LUA_NUMBER_DOUBLE
  uint64_t b;
#else
  UBits b;
#endif
} BitNum;

typedef union {
    lua_Number n;
    uint64_t b;
} BitNum64;

/* Convert argument to bit type. */
static UBits barg(lua_State *L, int idx)
{
  BitNum bn;
  UBits b;
#if LUA_VERSION_NUM < 502
  bn.n = lua_tonumber(L, idx);
#else
  bn.n = luaL_checknumber(L, idx);
#endif
#if defined(LUA_NUMBER_DOUBLE)
  bn.n += 6755399441055744.0;  /* 2^52+2^51 */
#ifdef SWAPPED_DOUBLE
  b = (UBits)(bn.b >> 32);
#else
  b = (UBits)bn.b;
#endif
#elif defined(LUA_NUMBER_INT) || defined(LUA_NUMBER_LONG) || \
      defined(LUA_NUMBER_LONGLONG) || defined(LUA_NUMBER_LONG_LONG) || \
      defined(LUA_NUMBER_LLONG)
  if (sizeof(UBits) == sizeof(lua_Number))
    b = bn.b;
  else
    b = (UBits)(SBits)bn.n;
#elif defined(LUA_NUMBER_FLOAT)
#error "A 'float' lua_Number type is incompatible with this library"
#else
#error "Unknown number type, check LUA_NUMBER_* in luaconf.h"
#endif
#if LUA_VERSION_NUM < 502
  if (b == 0 && !lua_isnumber(L, idx)) {
    luaL_typerror(L, idx, "number");
  }
#endif
  return b;
}

static UBits64 barg64(lua_State *L, int idx)
{
#if LUA_VERSION_NUM < 502
    lua_Number n = lua_tonumber(L, idx);
#else
    lua_Number n = luaL_checknumber(L, idx);
#endif

    return (UBits64)n;
}

/* Return bit type. */
#define BRET(b)  lua_pushnumber(L, (lua_Number)(SBits)(b)); return 1;
#define BRET64(b) \
    lua_pushnumber(L, (lua_Number)(SBits64)(b)); \
    return 1;

static int bit_tobit(lua_State *L) { BRET(barg(L, 1)) }
static int bit64_tobit(lua_State *L)
{
    BRET64(barg64(L, 1))
}
static int bit_bnot(lua_State *L) { BRET(~barg(L, 1)) }
static int bit64_bnot(lua_State *L)
{
    BRET64(~barg64(L, 1))
}

#define BIT_OP(func, opr) \
  static int func(lua_State *L) { int i; UBits b = barg(L, 1); \
    for (i = lua_gettop(L); i > 1; i--) b opr barg(L, i); BRET(b) }
BIT_OP(bit_band, &=)
BIT_OP(bit_bor, |=)
BIT_OP(bit_bxor, ^=)

#define bshl(b, n)  (b << n)
#define bshr(b, n)  (b >> n)
#define bsar(b, n)  ((SBits)b >> n)
#define brol(b, n)  ((b << n) | (b >> (32-n)))
#define bror(b, n)  ((b << (32-n)) | (b >> n))
#define BIT_SH(func, fn) \
  static int func(lua_State *L) { \
    UBits b = barg(L, 1); UBits n = barg(L, 2) & 31; BRET(fn(b, n)) }
BIT_SH(bit_lshift, bshl)
BIT_SH(bit_rshift, bshr)
BIT_SH(bit_arshift, bsar)
BIT_SH(bit_rol, brol)
BIT_SH(bit_ror, bror)

  #define BIT64_OP(func, opr) \
static int func(lua_State *L) { \
    int i; \
    UBits64 b = barg64(L, 1); \
    for (i = lua_gettop(L); i > 1; i--) \
        b opr barg64(L, i); \
    BRET64(b) \
}

BIT64_OP(bit64_band, &=)
BIT64_OP(bit64_bor, |=)
BIT64_OP(bit64_bxor, ^=)
  
#define bshl64(b, n) ((b) << (n))
#define bshr64(b, n) ((b) >> (n))
#define bsar64(b, n) ((SBits64)(b) >> (n))

#define brol64(b, n) (((b) << (n)) | ((b) >> (64 - (n))))
#define bror64(b, n) (((b) << (64 - (n))) | ((b) >> (n)))

#define BIT64_SH(func, fn) \
static int func(lua_State *L) { \
    UBits64 b = barg64(L, 1); \
    UBits64 n = barg64(L, 2) & 63; \
    BRET64(fn(b, n)) \
}

BIT64_SH(bit64_lshift, bshl64)
BIT64_SH(bit64_rshift, bshr64)
BIT64_SH(bit64_arshift, bsar64)
BIT64_SH(bit64_rol, brol64)
BIT64_SH(bit64_ror, bror64)
  
static int bit_bswap(lua_State *L)
{
  UBits b = barg(L, 1);
  b = (b >> 24) | ((b >> 8) & 0xff00) | ((b & 0xff00) << 8) | (b << 24);
  BRET(b)
}

static int bit64_bswap(lua_State *L)
{
    UBits64 b = barg64(L, 1);

    b =
        ((b & 0x00000000000000FFULL) << 56) |
        ((b & 0x000000000000FF00ULL) << 40) |
        ((b & 0x0000000000FF0000ULL) << 24) |
        ((b & 0x00000000FF000000ULL) << 8 ) |
        ((b & 0x000000FF00000000ULL) >> 8 ) |
        ((b & 0x0000FF0000000000ULL) >> 24) |
        ((b & 0x00FF000000000000ULL) >> 40) |
        ((b & 0xFF00000000000000ULL) >> 56);

    BRET64(b)
}

static int bit_tohex(lua_State *L)
{
  UBits b = barg(L, 1);
  SBits n = lua_isnone(L, 2) ? 8 : (SBits)barg(L, 2);
  const char *hexdigits = "0123456789abcdef";
  char buf[8];
  int i;
  if (n < 0) { n = -n; hexdigits = "0123456789ABCDEF"; }
  if (n > 8) n = 8;
  for (i = (int)n; --i >= 0; ) { buf[i] = hexdigits[b & 15]; b >>= 4; }
  lua_pushlstring(L, buf, (size_t)n);
  return 1;
}

static int bit64_tohex(lua_State *L)
{
    UBits64 b = barg64(L, 1);

    const char *hex = "0123456789abcdef";

    char buf[16];
    int i;

    for (i = 15; i >= 0; --i) {
        buf[i] = hex[b & 0xF];
        b >>= 4;
    }

    lua_pushlstring(L, buf, 16);
    return 1;
}

static int bit_tobin(lua_State *L)
{
    UBits b = barg(L, 1);

    char buf[33];
    int i;

    for (i = 31; i >= 0; --i)
        buf[31 - i] = (b & (1u << i)) ? '1' : '0';

    buf[32] = '\0';

    lua_pushstring(L, buf);
    return 1;
}

static int bit64_tobin(lua_State *L)
{
    UBits64 b = barg64(L, 1);

    char buf[65];
    int i;

    for (i = 63; i >= 0; --i)
        buf[63 - i] = (b & ((UBits64)1 << i)) ? '1' : '0';

    buf[64] = '\0';

    lua_pushstring(L, buf);
    return 1;
}

static const struct luaL_Reg bit_funcs[] = {
  { "tobit",	bit_tobit },
  { "bnot",	bit_bnot },
  { "band",	bit_band },
  { "bor",	bit_bor },
  { "bxor",	bit_bxor },
  { "lshift",	bit_lshift },
  { "rshift",	bit_rshift },
  { "arshift",	bit_arshift },
  { "rol",	bit_rol },
  { "ror",	bit_ror },
  { "bswap",	bit_bswap },
  { "tohex",	bit_tohex },
  { "tobin",  bit_tobin },
  { NULL, NULL }
};

static const struct luaL_Reg bit64_funcs[] = {
    { "tobit", bit64_tobit },
    { "bnot", bit64_bnot },
    { "band", bit64_band },
    { "bor", bit64_bor },
    { "bxor", bit64_bxor },
    { "lshift", bit64_lshift },
    { "rshift", bit64_rshift },
    { "arshift", bit64_arshift },
    { "rol", bit64_rol },
    { "ror", bit64_ror },
    { "bswap", bit64_bswap },
    { "tohex", bit64_tohex },
    { "tobin", bit64_tobin },
    { NULL, NULL }
};

/* Signed right-shifts are implementation-defined per C89/C99.
** But the de facto standard are arithmetic right-shifts on two's
** complement CPUs. This behaviour is required here, so test for it.
*/
#define BAD_SAR		(bsar(-8, 2) != (SBits)-2)

LUALIB_API int luaopen_bit(lua_State *L)
{
  UBits b;
  lua_pushnumber(L, (lua_Number)1437217655L);
  b = barg(L, -1);
  if (b != (UBits)1437217655L || BAD_SAR) {  /* Perform a simple self-test. */
    const char *msg = "compiled with incompatible luaconf.h";
#ifdef LUA_NUMBER_DOUBLE
#ifdef _WIN32
    if (b == (UBits)1610612736L)
      msg = "use D3DCREATE_FPU_PRESERVE with DirectX";
#endif
    if (b == (UBits)1127743488L)
      msg = "not compiled with SWAPPED_DOUBLE";
#endif
    if (BAD_SAR)
      msg = "arithmetic right-shift broken";
    luaL_error(L, "bit library self-test failed (%s)", msg);
  }
#if LUA_VERSION_NUM < 502
  luaL_register(L, "bit", bit_funcs);
#else
  luaL_newlib(L, bit_funcs);
#endif
  return 1;
}

LUALIB_API int luaopen_bit64(lua_State *L)
{
#if LUA_VERSION_NUM < 502
    luaL_register(L, "bit64", bit64_funcs);
#else
    luaL_newlib(L, bit64_funcs);
#endif

    return 1;
}

