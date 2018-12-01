
#ifndef __DMLUA_OPT_H_INCLUDE__
#define __DMLUA_OPT_H_INCLUDE__

#include <stdlib.h>
#include "dmlua.h"

struct lua_nil {};
// 如果C为true则是A类型，false是B类型
template <bool C, typename A, typename B>	struct if_ {};
template <typename A, typename B>			struct if_<true, A, B> { typedef A type; };
template <typename A, typename B>			struct if_<false, A, B> { typedef B type; };

// 判断是否指针
template <typename T>
struct is_ptr { static const bool value = false; };
template <typename T>
struct is_ptr<T*> { static const bool value = true; };

// 判断是否是引用
template<typename T>
struct is_ref { static const bool value = false; };
template<typename T>
struct is_ref<T&> { static const bool value = true; };

// 获取基本类型 指针以及引用
template <typename T>
struct base_type { typedef T type; };
template <typename T>
struct base_type<T*> { typedef T type; };
template <typename T>
struct base_type<T&> { typedef T type; };

// 输入参数转成T类型
template<typename T>
struct void2val { static T invoke(void* input) { return *(T*)input; } };
// 输入参数转成T类型指针
template<typename T>
struct void2ptr { static T* invoke(void* input) { return (T*)input; } };
// 输入参数转成T类型引用
template<typename T>
struct void2ref { static T& invoke(void* input) { return *(T*)input; } };

// 将输入参数ptr转换成T T* 或者T&
template <typename T>
struct void2type {
	static T invoke(void* ptr) {
		return if_<is_ptr<T>::value,
					void2ptr<typename base_type<T>::type>,
					typename if_<is_ref<T>::value,
						void2ref<typename base_type<T>::type>,
						void2val<typename base_type<T>::type>
					>::type
				>::type::invoke(ptr);
	}
};

template <typename T>
struct LuaReader
{
    static inline T Read(lua_State* L, int index)
    {
        if (CluaTypeid::Instance().get_name<T>()) {
            return void2type<T>::invoke(tolua_tousertype(L, index, nullptr));
        }
        else {
            return void2type<T>::invoke(tolua_touserdata(L, index, nullptr));
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

//static inline void PushLuaParam(lua_State* luaS, const char* t) {
//    tolua_pushstring(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const bool t) {
//    tolua_pushboolean(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, std::string& t) {
//    tolua_pushstring(luaS, t.c_str());
//}
//
//static inline void PushLuaParam(lua_State* luaS, const std::string& t) {
//    tolua_pushstring(luaS, t.c_str());
//}
//
//static inline void PushLuaParam(lua_State* luaS, const short t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const unsigned short t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const int t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const unsigned int t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const long t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const unsigned long t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const long long t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const unsigned long long t) {
//    lua_pushinteger(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const double t) {
//    tolua_pushnumber(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const char* t) {
//    tolua_pushstring(luaS, t);
//}
//
//static inline void PushLuaParam(lua_State* luaS, const bool t) {
//    tolua_pushboolean(luaS, t);
//}

template <typename T>
static inline T LuaPop(lua_State* L) { T ret = LuaReader<T>::Read(L, -1); lua_pop(L, 1); return ret; }

static inline void LuaPop(lua_State* L) { lua_pop(L, 1); }

#endif // __DMLUA_OPT_H_INCLUDE__

