
#ifndef __DMLUA_LUARESULT_H_INCLUDE__
#define __DMLUA_LUARESULT_H_INCLUDE__

#include <string>
#include <stdint.h>

template<typename T>
struct LuaResult
{
    LuaResult() : value(T()){}
    LuaResult(const T& t) : value(t){}
    operator T& (){ return value; }
    T value;
};

template<>
struct LuaResult<std::string>
{
    LuaResult() : value(std::string()){}
    operator const char* (){ return value.c_str(); }
    std::string value;
};

template<>
struct LuaResult<long long>
{
    LuaResult() : value(0) {}
    LuaResult(const long long& t) : value(t) {}
    operator long long() { return value; }
    int64_t value;
};

template<>
struct LuaResult<unsigned long long>
{
    LuaResult() : value(0) {}
    LuaResult(const unsigned long long& t) : value(t) {}
    operator unsigned long long() { return value; }
    unsigned long long value;
};

typedef LuaResult<int>                  LResultINT;
typedef LuaResult<long long>            LResultINT64;
typedef LuaResult<unsigned long long>   LResultUINT64;
typedef LuaResult<double>               LResultDOUBLE;

typedef LuaResult<std::string>          LResultSTRING;

#endif // __DMLUA_LUARESULT_H_INCLUDE__
