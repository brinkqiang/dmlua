
#ifndef __DMLUA_OPT_H_INCLUDE__
#define __DMLUA_OPT_H_INCLUDE__

#include <stdlib.h>
#include "dmlua.h"

struct lua_nil {};
// ���CΪtrue����A���ͣ�false��B����
template <bool C, typename A, typename B>	struct if_ {};
template <typename A, typename B>			struct if_<true, A, B> { typedef A type; };
template <typename A, typename B>			struct if_<false, A, B> { typedef B type; };

// �ж��Ƿ�ָ��
template <typename T>
struct is_ptr { static const bool value = false; };
template <typename T>
struct is_ptr<T*> { static const bool value = true; };

// �ж��Ƿ�������
template<typename T>
struct is_ref { static const bool value = false; };
template<typename T>
struct is_ref<T&> { static const bool value = true; };

// ��ȡ�������� ָ���Լ�����
template <typename T>
struct base_type { typedef T type; };
template <typename T>
struct base_type<T*> { typedef T type; };
template <typename T>
struct base_type<T&> { typedef T type; };

// �������ת��T����
template<typename T>
struct void2val { static T invoke(void* input) { return *(T*)input; } };
// �������ת��T����ָ��
template<typename T>
struct void2ptr { static T* invoke(void* input) { return (T*)input; } };
// �������ת��T��������
template<typename T>
struct void2ref { static T& invoke(void* input) { return *(T*)input; } };

// ���������ptrת����T T* ����T&
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

//template <typename T>
//T   LuaReader(lua_State* L, int index) {
//	if (CluaTypeid::Instance().get_name<T>()) {
//		return void2type<T>::invoke(tolua_tousertype(L, index, nullptr));
//	}
//	else {
//		return void2type<T>::invoke(tolua_touserdata(L, index, nullptr));
//	}
//}

//template <>	inline void LuaReader(lua_State* L, int index)
//{
//    return;
//}
//
//template <>	inline void* LuaReader(lua_State* L, int index)
//{
//    return tolua_touserdata(L, index, nullptr);
//}
//
//template <> inline char* LuaReader(lua_State* L, int index)
//{
//    return const_cast<char*>(tolua_tostring(L, index, ""));
//}
//
//template <> inline const char* LuaReader(lua_State* L, int index)
//{
//    return tolua_tostring(L, index, "");
//}
//
//template <> inline std::string LuaReader(lua_State* L, int index)
//{
//    return tolua_tocppstring(L, index, "");
//}
//
//template <> inline int8_t LuaReader(lua_State* L, int index)
//{
//    return (int8_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline uint8_t LuaReader(lua_State* L, int index)
//{
//    return (uint8_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline int16_t LuaReader(lua_State* L, int index)
//{
//    return (int16_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline uint16_t LuaReader(lua_State* L, int index)
//{
//    return (uint16_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline int32_t LuaReader(lua_State* L, int index)
//{
//    return (int32_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline uint32_t LuaReader(lua_State* L, int index)
//{
//    return (uint32_t)tolua_tointeger(L, index, 0);
//}
//
//template <> inline int64_t LuaReader(lua_State* L, int index)
//{
//    return tolua_tointeger(L, index, 0);
//}
//
//template <> inline uint64_t LuaReader(lua_State* L, int index)
//{
//    return tolua_tointeger(L, index, 0);
//}
//
//template <> inline bool LuaReader(lua_State* L, int index)
//{
//    if (lua_isnil(L, index)) { return false; }
//    if (lua_isboolean(L, index)) {
//        return tolua_toboolean(L, index, 0) != 0;
//    }
//    else {
//        return tolua_tonumber(L, index, 0) != 0;
//    }
//}
//
//template <> inline float LuaReader(lua_State* L, int index)
//{
//    return (float)tolua_tonumber(L, index, 0);
//}
//
//template <> inline double LuaReader(lua_State* L, int index)
//{
//    return (double)tolua_tonumber(L, index, 0);
//}

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
struct LuaReader<int64_t>
{
    static inline int64_t Read(lua_State* L, int index)
    {
        return tolua_tointeger(L, index, 0);
    }
};

template <>
struct LuaReader<uint64_t>
{
    static inline uint64_t Read(lua_State* L, int index)
    {
        return tolua_tointeger(L, index, 0);
    }
};

template <typename T>
static inline T LuaPop(lua_State* L) { T ret = LuaReader<T>::Read(L, -1); lua_pop(L, 1); return ret; }

static inline void LuaPop(lua_State* L) { lua_pop(L, 1); }

#endif // __DMLUA_OPT_H_INCLUDE__

