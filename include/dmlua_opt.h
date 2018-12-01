
#ifndef __DMLUA_OPT_H_INCLUDE__
#define __DMLUA_OPT_H_INCLUDE__

#include <stdlib.h>
#include "dmlua.h"

template <typename T>
struct LuaReader
{
    static inline T Read(lua_State* L, int index)
    {
        if (CluaTypeid::Instance().get_name<T>()) {
            return (T)tolua_tousertype(L, index, CluaTypeid::Instance().get_name<T>());
        }
        else {
            return (T)tolua_touserdata(L, index, NULL);
        }
    }
};

template <>
struct LuaReader<void>
{
    static inline void Read(lua_State* L, int index)
    {

    }
};

template <>
struct LuaReader<void*>
{
    static inline void* Read(lua_State* L, int index)
    {
        return tolua_touserdata(L, index, NULL);
    }
};

template <>
struct LuaReader<char>
{
    static inline char Read(lua_State* L, int index)
    {
        return (char)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<unsigned char>
{
    static inline unsigned char Read(lua_State* L, int index)
    {
        return (unsigned char)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<short>
{
    static inline short Read(lua_State* L, int index)
    {
        return (short)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<unsigned short>
{
    static inline unsigned short Read(lua_State* L, int index)
    {
        return (unsigned short)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<int>
{
    static inline int Read(lua_State* L, int index)
    {
        return (int)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<unsigned int>
{
    static inline unsigned int Read(lua_State* L, int index)
    {
        return (unsigned int)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<long>
{
    static inline long Read(lua_State* L, int index)
    {
        return (long)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<unsigned long>
{
    static inline unsigned long Read(lua_State* L, int index)
    {
        return (unsigned long)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<long long>
{
    static inline long long Read(lua_State* L, int index)
    {
        return (long long)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<unsigned long long>
{
    static inline unsigned long long Read(lua_State* L, int index)
    {
        return (unsigned long long)tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<float>
{
    static inline float Read(lua_State* L, int index)
    {
        return (float)tolua_tonumber(L, index, 0);
    }
};

template <>
struct LuaReader<double>
{
    static inline double Read(lua_State* L, int index)
    {
        return (double)tolua_tonumber(L, index, 0);
    }
};

template <>
struct LuaReader<const char*>
{
    static inline const char* Read(lua_State* L, int index)
    {
        return tolua_tostring(L, index, "");
    }
};

template <>
struct LuaReader<char*>
{
    static inline char* Read(lua_State* L, int index)
    {
        return const_cast<char*>(tolua_tostring(L, index, ""));
    }
};

template <>
struct LuaReader<bool>
{
    static inline bool Read(lua_State* L, int index)
    {
        if (lua_isnil(L, index)) { return false; }
        if (lua_isboolean(L, index)) {
            return tolua_toboolean(L, index, 0) != 0;
        }
        else {
            return tolua_tonumber(L, index, 0) != 0;
        }
    }
};

template <typename T>
static inline T LuaPop(lua_State* L) { T ret = LuaReader<T>::Read(L, -1); lua_pop(L, 1); return ret; }

static inline void LuaPop(lua_State* L) { lua_pop(L, 1); }

#endif // __DMLUA_OPT_H_INCLUDE__

