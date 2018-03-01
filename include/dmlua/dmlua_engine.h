
#ifndef __DMLUA_ENGINE_H_INCLUDE__
#define __DMLUA_ENGINE_H_INCLUDE__

#include <assert.h>
#include <string.h>
#include <string>
#include <vector>
#include <algorithm>
#include <typeinfo>

#include "lua.hpp"
#include "tolua++.h"

#include "dmlua_types.h"

#include "dmlua_parser.h"
#include "dmlua_typeid.h"
#include "dmlua_luaresult.h"

#include "safesingleton.h"

TOLUA_API int tolua_interface_open (lua_State* tolua_S);

inline char* tolua_SafeStrCopy(char *des, const char *src, size_t max_len)
{
    // initialize for check below
    if(NULL == src)
    {
        des[0] = '\0';
        return des;
    }
    des[max_len-1] = 0;
    return strnlen(src,max_len) < max_len ? strcpy(des, src) : strncpy(des, src, max_len-1);
}

template <size_t N>
inline char* tolua_SafeStrCopy(char (&des)[N], const char* src)
{
    return tolua_SafeStrCopy(des, src, sizeof(des));
}

template <size_t N>
inline void tolua_ZeroString(char (&des)[N]) throw()
{
    des[0] = '\0';
    des[sizeof(des)-1] = '\0';
}

template <size_t N>
inline void tolua_SafeSprintf(char (&des)[N], const char *format, ...) throw()
{
    va_list args;
    va_start(args,format);
    vsnprintf(des, sizeof(des)-1, format, args);
    va_end(args);
    des[sizeof(des)-1] = '\0';
}

#ifdef WIN32
inline size_t tolua_StrNLen(const char *src, size_t max_len)
{
    size_t i;
    const char *ptr = src;

    for(i = 0; i < max_len; i++)
    {
        if('\0' == *ptr)
        {
            return i;
        }
        ptr++;
    }

    return max_len;
}
#else
inline size_t tolua_StrNLen(const char *src, size_t max_len)
{
    return strnlen(src, max_len);
}
#endif

inline char *
    tolua_strtok_r(char *ptr, const char *sep, char **end)
{
    if(!ptr)
        /* we got NULL input so then we get our last position instead */
            ptr = *end;

    /* pass all letters that are including in the separator string */
    while(*ptr && strchr(sep, *ptr))
        ++ptr;

    if(*ptr) {
        /* so this is where the next piece of string starts */
        char *start = ptr;
        /* set the end pointer to the first byte after the start */
        *end = start + 1;

        /* scan through the string to find where it ends, it ends on a
        null byte or a character that exists in the separator string */
        while(**end && !strchr(sep, **end))
            ++*end;

        if(**end) {
            /* the end is not a null byte */
            **end = '\0';  /* zero terminate it! */
            ++*end;        /* advance the last pointer to beyond the null byte */
        }

        return start; /* return the position where the string starts */
    }

    /* we ended up on a null byte, there are no more strings to find! */
    return NULL;
}

#ifdef WIN32

struct __timezone
{
    int  tz_minuteswest; /* minutes W of Greenwich */
    int  tz_dsttime;     /* type of dst correction */
};

struct __timeval {
    long    tv_sec;         /* seconds */
    long    tv_usec;        /* and microseconds */
};

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif

typedef union {
    unsigned long long ft_scalar;
    FILETIME ft_struct;
} FT;

static inline int gettimeofday(struct __timeval *tv, struct __timezone *tz)
{
    FT ft;
    static int tzflag = 0;

    if (NULL != tv)
    {
        GetSystemTimeAsFileTime(&ft.ft_struct);
        ft.ft_scalar /= 10;
        ft.ft_scalar -= DELTA_EPOCH_IN_MICROSECS;
        tv->tv_sec = (long)(ft.ft_scalar / 1000000UL);
        tv->tv_usec = (long)(ft.ft_scalar % 1000000UL);
    }

    if (NULL != tz)
    {
        if (!tzflag)
        {
            _tzset();
            tzflag++;
        }

        tz->tz_minuteswest = _timezone / 60;
        tz->tz_dsttime = _daylight;
    }

    return 0;
}

#endif

inline unsigned int GetTickCount32()
{
#ifdef WIN32
    struct __timeval tv = {0};
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#else
    struct timeval tv = {0};
    gettimeofday(&tv,NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif
}

#define LUA_CHECK_FUNCTION(luaS, func) \
    CLuaStateGuard oGuard(luaS, func);\
    do{ if (LuaCheckFunction(luaS, func)){ OnUserError(luaS, func); return -1;} }while(0)

#define LUA_CALL_FUNCTION(luaS, func, arg, res) \
    do{ OnEventLuaCallStart();if (lua_pcall(luaS, arg, res, 0)){ OnEventLuaCallEnd();OnScriptError(luaS, func);return -1;}; OnEventLuaCallEnd();} while(0)

#define LUA_CALL_FUNCTION_NOEVENT(luaS, func, arg, res) \
    do{ if (lua_pcall(luaS, arg, res, 0)){ OnScriptError(luaS, func);return -1;}; } while(0)

class CLuaStateGuard
{

public:
    explicit CLuaStateGuard(lua_State* pLuaS, const char *func)
        : m_pLuaS(pLuaS), m_func(func)
    {
        m_nTop = lua_gettop(m_pLuaS);
        //if (m_nTop)
        //{
        //    fprintf(stderr, "function: [%s] found top: [%d] error\n", m_func, m_nTop);
        //}
    }

    ~CLuaStateGuard()
    {
        lua_settop(m_pLuaS,  m_nTop);
    }
private:
    lua_State*    m_pLuaS;
    int           m_nTop;
    const char*   m_func;
};

class CLuaFunctionExtractQuick
{
public:
    CLuaFunctionExtractQuick(const char*func, const char*sep)
        : m_pSep(sep), m_pEndNode(NULL)
    {
        tolua_SafeStrCopy(szFunc, func);
    }

    char* First()
    {
        char* pStartNode =  tolua_strtok_r(szFunc, m_pSep, &m_pEndNode);
        if (NULL == pStartNode)
        {
            return NULL;
        }

        return pStartNode;
    }

    char* Next()
    {
        char* pStartNode =  tolua_strtok_r(NULL, m_pSep, &m_pEndNode);
        if (NULL == pStartNode)
        {
            return NULL;
        }

        return pStartNode;
    }
private:
    char        szFunc[MAX_PATH];
    char*       m_pEndNode;
    const char* m_pSep;
};

class CLuaFunctionExtract
{
public:
    CLuaFunctionExtract(const char*func,const char*sep)
        : m_pSep(sep), m_pEndNode(NULL)
    {
        strFunc.assign(func);
    }

    char* first()
    {
        char* pStartNode =  tolua_strtok_r(&strFunc[0], m_pSep, &m_pEndNode);
        if (NULL == pStartNode)
        {
            return NULL;
        }

        return pStartNode;
    }

    char* next()
    {
        char* pStartNode =  tolua_strtok_r(NULL, m_pSep, &m_pEndNode);
        if (NULL == pStartNode)
        {
            return NULL;
        }

        return pStartNode;
    }
private:
    std::string strFunc;
    char*       m_pEndNode;
    const char* m_pSep;
};

// tolua_begin
// tolua_end
class CDMLuaEngine : public HSafeSingleton<CDMLuaEngine>
{
    friend class HSafeSingleton<CDMLuaEngine>;
public:

    typedef struct tagFileInfo
    {
        std::string strFullPathFile;
        std::string strFullPath;
        std::string strModuleName;
    }SFileInfo;

public:
    CDMLuaEngine()
        : m_pLuaS(luaL_newstate()), m_dwStartTime(0), m_bStartTime(false)
    {
        luaL_openlibs(m_pLuaS);
        tolua_interface_open(m_pLuaS);
    }

    virtual ~CDMLuaEngine()
    {
        if (m_pLuaS)
        {
            lua_close(m_pLuaS);
            m_pLuaS = NULL;
        }
    }

    void SetRootPath(const std::string& strPath)
    {
        m_strSrcPath = strPath;
    }

    void Swap(CDMLuaEngine& oEngine)
    {
        std::swap(m_pLuaS, oEngine.m_pLuaS);
        std::swap(m_vecFileInfo, oEngine.m_vecFileInfo);
        std::swap(m_strSrcPath, oEngine.m_strSrcPath);
        std::swap(m_strCwd, oEngine.m_strCwd);
        std::swap(m_dwStartTime, oEngine.m_dwStartTime);

        //RegisterEvent(*this);
        //RegisterEvent(oEngine);
    }

    void RunLuagc()
    {
        lua_gc(m_pLuaS, LUA_GCCOLLECT, 0);
    }

    long long GetLuagc()
    {
        return lua_gc(m_pLuaS, LUA_GCCOUNT, 0) * 1024L + lua_gc(m_pLuaS, LUA_GCCOUNTB, 0);
    }

    lua_State* GetState(){ return m_pLuaS; }

    bool DoString(const char* data)
    {
        CLuaStateGuard oGuard(m_pLuaS, "DoString");
        int nRet = luaL_dostring(m_pLuaS, data);
        if (0 != nRet)
        {
            OnScriptError(m_pLuaS, "DoString");
        }

        return 0 == nRet ? true : false;
    }
	bool DoFile(const char* data)
	{
		CLuaStateGuard oGuard(m_pLuaS, "DoFile");
		int nRet = luaL_dofile(m_pLuaS, data);
		if (0 != nRet)
		{
			OnScriptError(m_pLuaS, "DoFile");
		}

		return 0 == nRet ? true : false;
	}
    bool LoadScript(const std::string& strName)
    {
        __ParserBegin();
        __LoadScript(strName);
        __ParserEnd();
        return true;
    }

    bool LoadScript()
    {
        __SetSrcDirectory(!m_strSrcPath.empty() ? m_strSrcPath : __GetScriptPath());
        __ParserBegin();
        __ParserFiles(m_strSrcPath, "");
        __LoadScriptAll();
        __ParserEnd();

        return true;
    }

    bool ReloadScript()
    {
        CDMLuaEngine oEngine;

        if (!oEngine.LoadScript())
        {
            goto FAIL;
        }

        oEngine.Swap(*this);
        return true;
FAIL:
        return false;
    }

    static inline int LuaCheckFunction(lua_State *luaS, const char *func)
    {
        CLuaFunctionExtractQuick oLuaExtract(func, ".");

        char* start = oLuaExtract.First();
        if (NULL == start){ return -1;}
        char* next = oLuaExtract.Next();
        if (NULL == next){ return tolua_isGlobalFunction(luaS, start);}
        if (tolua_isGlobalTable(luaS, start)){ return -1;}
        char* end = oLuaExtract.Next();
        if (NULL == end)
        {
            return tolua_isFunction(luaS, next);
        }

        do
        {
            if (tolua_isTable(luaS, next)){ return -1;}
            next = end;
            end = oLuaExtract.Next();
        } while (end);
        if (tolua_isFunction(luaS, next)){ return -1;}

        return 0;
    }

    template<typename T>
    static inline void PushLuaParam(lua_State *luaS, T &t)
    {
        if (CluaTypeid::Instance().get_name<T>()){tolua_pushusertype(luaS, (void*)(&t), CluaTypeid::Instance().get_name<T>());}
        else{ tolua_pushuserdata(luaS, (void*)(&t));OnUserError(luaS, typeid(T).name());}
    }

    template<typename T>
    static inline void PushLuaParam(lua_State *luaS, T* &t)
    {
        if (CluaTypeid::Instance().get_name<T>()){tolua_pushusertype(luaS, t, CluaTypeid::Instance().get_name<T>());}
        else{ tolua_pushuserdata(luaS, (void*)(t));OnUserError(luaS, typeid(T).name());}
    }

    static inline void PushLuaParam(lua_State *luaS, const std::string &t)
    {
        tolua_pushstring(luaS, t.c_str());
    }

    static inline void PushLuaParam(lua_State *luaS, const short t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const unsigned short t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const int t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const unsigned int t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const long t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const unsigned long t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const long long t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const unsigned long long t)
    {
        lua_pushinteger(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const double t)
    {
        tolua_pushnumber(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const char* t)
    {
        tolua_pushstring(luaS, t);
    }

    static inline void PushLuaParam(lua_State *luaS, const bool t)
    {
        tolua_pushboolean(luaS, t);
    }

    //////////////////////////////////////////////////////////////////////////

    static inline int tolua_isGlobalTable(lua_State *luaS, const char *func)
    {
        lua_pushglobaltable(luaS);
        lua_getfield(luaS, -1, func);
        if (lua_istable(luaS, -1))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    static inline int tolua_isFunction(lua_State *luaS, const char *func)
    {
        lua_getfield(luaS, -1, func);
        if (lua_isfunction(luaS, -1))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    static inline int tolua_isTable(lua_State *luaS, const char *func)
    {
        lua_getfield(luaS, -1, func);
        if (lua_istable(luaS, -1))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    static inline int tolua_isGlobalFunction(lua_State *luaS, const char *func)
    {
        lua_pushglobaltable(luaS);
        lua_getfield(luaS, -1, func);
        if (lua_isfunction(luaS, -1))
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }

    inline int Call(const char *func)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);

        LUA_CALL_FUNCTION(m_pLuaS, func, 0, 0);
        return 0;
    }

    template<typename T1>
    inline int Call(const char *func, T1 t1)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);

        LUA_CALL_FUNCTION(m_pLuaS, func, 1, 0);
        return 0;
    }

    template<typename T1,typename T2>
    inline int Call(const char *func, T1 t1, T2 t2)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);

        LUA_CALL_FUNCTION(m_pLuaS, func, 2, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);

        LUA_CALL_FUNCTION(m_pLuaS, func, 3, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);

        LUA_CALL_FUNCTION(m_pLuaS, func, 4, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5 )
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);

        LUA_CALL_FUNCTION(m_pLuaS, func, 5, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6 )
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);
        PushLuaParam(m_pLuaS, t6);

        LUA_CALL_FUNCTION(m_pLuaS, func, 6, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7 )
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);
        PushLuaParam(m_pLuaS, t6);
        PushLuaParam(m_pLuaS, t7);

        LUA_CALL_FUNCTION(m_pLuaS, func, 7, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8 )
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);
        PushLuaParam(m_pLuaS, t6);
        PushLuaParam(m_pLuaS, t7);
        PushLuaParam(m_pLuaS, t8);

        LUA_CALL_FUNCTION(m_pLuaS, func, 8, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9 )
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);
        PushLuaParam(m_pLuaS, t6);
        PushLuaParam(m_pLuaS, t7);
        PushLuaParam(m_pLuaS, t8);
        PushLuaParam(m_pLuaS, t9);

        LUA_CALL_FUNCTION(m_pLuaS, func, 9, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
    inline int Call(const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10)
    {
        LUA_CHECK_FUNCTION(m_pLuaS, func);
        PushLuaParam(m_pLuaS, t1);
        PushLuaParam(m_pLuaS, t2);
        PushLuaParam(m_pLuaS, t3);
        PushLuaParam(m_pLuaS, t4);
        PushLuaParam(m_pLuaS, t5);
        PushLuaParam(m_pLuaS, t6);
        PushLuaParam(m_pLuaS, t7);
        PushLuaParam(m_pLuaS, t8);
        PushLuaParam(m_pLuaS, t9);
        PushLuaParam(m_pLuaS, t10);

        LUA_CALL_FUNCTION(m_pLuaS, func, 10, 0);
        return 0;
    }

    //////////////////////////////////////////////////////////////////////////
    static inline int StaticCall(lua_State *luaS, const char *func)
    {
        LUA_CHECK_FUNCTION(luaS, func);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 0, 0);
        return 0;
    }

    template<typename T1>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1)
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 1, 0);
        return 0;
    }

    template<typename T1,typename T2>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2)
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 2, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3)
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 3, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4)
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 4, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5 )
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 5, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6 )
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);
        PushLuaParam(luaS, t6);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 6, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7 )
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);
        PushLuaParam(luaS, t6);
        PushLuaParam(luaS, t7);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 7, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8 )
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);
        PushLuaParam(luaS, t6);
        PushLuaParam(luaS, t7);
        PushLuaParam(luaS, t8);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 8, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8, typename T9>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9 )
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);
        PushLuaParam(luaS, t6);
        PushLuaParam(luaS, t7);
        PushLuaParam(luaS, t8);
        PushLuaParam(luaS, t9);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 9, 0);
        return 0;
    }

    template<typename T1,typename T2,typename T3,typename T4, typename T5, typename T6, typename T7, typename T8, typename T9, typename T10>
    static inline int StaticCall(lua_State *luaS, const char *func, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8, T9 t9, T10 t10)
    {
        LUA_CHECK_FUNCTION(luaS, func);
        PushLuaParam(luaS, t1);
        PushLuaParam(luaS, t2);
        PushLuaParam(luaS, t3);
        PushLuaParam(luaS, t4);
        PushLuaParam(luaS, t5);
        PushLuaParam(luaS, t6);
        PushLuaParam(luaS, t7);
        PushLuaParam(luaS, t8);
        PushLuaParam(luaS, t9);
        PushLuaParam(luaS, t10);

        LUA_CALL_FUNCTION_NOEVENT(luaS, func, 10, 0);
        return 0;
    }


private:
    static inline std::string __GetScriptPath()
    {
#ifdef WIN32
        static char path[MAX_PATH];
        static bool first_time = true;

        if(first_time)
        {
            first_time = false;
            GetModuleFileNameA(0, path, sizeof(path));
            char *p = strrchr(path, '\\');
            *(p) = '\0';
            p = strrchr(path, '\\');
            *(p) = '\0';
        }

        return path;
#else
        static char path[MAX_PATH];
        static bool first_time = true;

        if (first_time)
        {
            first_time = false;
            int nRet = readlink("/proc/self/exe", path, MAX_PATH);
            if ( nRet < 0 || nRet >= MAX_PATH )
            {
                return "./";
            }
            char *p = strrchr(path, '/');
            *(p) = '\0';
        }

        return path;
#endif
    }

    void __SetSrcDirectory(const std::string& strSrc){ m_strSrcPath = strSrc; }

    inline void __AddFile(const std::string& strPath, const std::string& strFullPathPwd, const std::string& strFullPath)
    {
        SFileInfo stFileInfo;
        stFileInfo.strFullPathFile = strPath;
        stFileInfo.strModuleName = strFullPathPwd;
        stFileInfo.strFullPath = strFullPath;
        m_vecFileInfo.push_back(stFileInfo);
    }

    inline void __ParserFiles(const std::string& strRoot, const std::string& strPwd)
    {
        CDirectoryParser oDirectoryParser;

        std::string strRealPath;

        if (strPwd.empty())
        {
            strRealPath = strRoot;
        }
        else
        {
#ifdef WIN32
            strRealPath = strRoot + "\\" + strPwd;
#else
            strRealPath = strRoot + "/" + strPwd;
#endif
        }

        if (!oDirectoryParser.Open(strRealPath.c_str()))
        {
            return;
        }

        do
        {
            char szPath[MAX_PATH] = {0};
            SFileAttr sAttr;
            memset(&sAttr, 0, sizeof(sAttr));
            if (!oDirectoryParser.Read(szPath, sizeof(szPath), &sAttr))
            {
                break;
            }

            std::string strFullPathPwd;

            if (strPwd.empty())
            {
                strFullPathPwd = szPath;
            }
            else
            {
#ifdef WIN32
                strFullPathPwd = strPwd + "\\" + szPath;
#else
                strFullPathPwd = strPwd + "/" + szPath;
#endif
            }

#ifdef WIN32
            std::string strFullPathFile = strRoot + "\\" + strFullPathPwd;
            std::string strFullPath = strRoot + "\\" + strPwd;
#else
            std::string strFullPathFile = strRoot + "/" + strFullPathPwd;
            std::string strFullPath = strRoot + "/" + strPwd;
#endif

            if (sAttr.isDir)
            {
                __ParserFiles(strRoot, strFullPathPwd);
                continue;
            }

            std::string ext = strFullPathFile.substr(strFullPathFile.rfind('.') == std::string::npos ? strFullPathFile.length() : strFullPathFile.rfind('.'));
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            if (ext == ".lua")
            {
                std::string strModuleName = strFullPathPwd.substr(0, strFullPathPwd.size() - ext.size());
#ifdef WIN32
                std::replace(strModuleName.begin(), strModuleName.end(),'\\', '.');
#else
                std::replace(strModuleName.begin(), strModuleName.end(), '/', '.');
#endif
                __AddFile(strFullPathFile , strModuleName, strFullPath);
            }
        } while(true);

        oDirectoryParser.Close();
        return;
    }

    void __ParserBegin()
    {
        char szPath[MAX_PATH] = {0};
        m_strCwd = getcwd(szPath, MAX_PATH);
        chdir(m_strSrcPath.c_str());
    }

    void __LoadScriptAll()
    {
        if (!m_vecFileInfo.empty())
        {
            for (int i = 0; i < static_cast<int>(m_vecFileInfo.size()); i++)
            {
                SFileInfo &sFileInfo = m_vecFileInfo[i];

                if (!__LoadScript(sFileInfo.strModuleName))
                {
                    ;
                }
            }
        }

        m_vecFileInfo.clear();
    }

    void __ParserEnd()
    {
        //int mask = LUA_MASKCALL | LUA_MASKRET | LUA_MASKLINE | LUA_MASKCOUNT;
        //lua_sethook(m_pLuaS, &FireLuaCheck, mask, 1);
        //chdir(m_strCwd.c_str());
    }

    bool __LoadScript(const std::string& strName)
    {
        CLuaStateGuard oGuard(m_pLuaS, strName.c_str());
        char szBuf[MAX_PATH] = {0};
        tolua_SafeSprintf(szBuf, "package.loaded[\"%s\"]=nil\n require \"%s\"\n", strName.c_str(), strName.c_str());

        int nRet = luaL_dostring(m_pLuaS, szBuf);
        if (0 != nRet)
        {
            OnScriptError(m_pLuaS, strName.c_str());
            return false;
        }
        else
        {
            return true;
        }
    }

    inline void OnEventLuaCallStart()
    {
        m_dwStartTime = GetTickCount32();
        m_bStartTime = true;
    }

    inline void OnEventLuaCallEnd()
    {
        m_dwStartTime = 0;
        m_bStartTime = false;
    }

    enum ECheckTime
    {
        eLuaCheckTime = 0,
    };

    inline void LuaCheck()
    {
        if (eLuaCheckTime && m_bStartTime)
        {
            unsigned int dwUseTime = GetTickCount32() - m_dwStartTime;

            if (dwUseTime > eLuaCheckTime)
            {
                luaL_error(m_pLuaS, "lua script timeout %d ms", dwUseTime);
            }
        }
    }

    static inline void OnUserError(lua_State* luaS, const char* name)
    {
        char szError[256];
        tolua_SafeSprintf(szError, "call Lua func error [%s] not exist\n", name);
        
        fprintf(stderr, szError);
    }

    static inline void OnScriptError(lua_State* luaS, const char* name)
    {
        char szError[256];
        tolua_SafeSprintf(szError, "call Lua script [%s] error [%s] \n", name, lua_tostring(luaS, -1));
        
        fprintf(stderr, szError);
    }

    //static void FireLuaCheck(lua_State* luaS, lua_Debug* ar)
    //{
    //    CDMLuaEngine* poEngine = FromLuaState(luaS);
    //    if (NULL != poEngine)
    //    {
    //        poEngine->LuaCheck();
    //    }
    //}

    //static CDMLuaEngine* FromLuaState(lua_State* luaS)
    //{
    //    return static_cast<CDMLuaEngine*>(__lua_getuserdata(luaS));
    //}

    //static void RegisterEvent(CDMLuaEngine& oEngine)
    //{
    //    assert(oEngine.GetState());

    //    __lua_setuserdata(oEngine.GetState(),  (void*)&oEngine);
    //}

protected:
    lua_State*    m_pLuaS;

    typedef std::vector<SFileInfo> VecFileInfo;
    typedef VecFileInfo::iterator VecFileInfoIt;

    VecFileInfo m_vecFileInfo;
    std::string m_strSrcPath;
    std::string m_strCwd;
    unsigned int m_dwStartTime;
    bool m_bStartTime;
};

#endif // __DMLUA_ENGINE_H_INCLUDE__
