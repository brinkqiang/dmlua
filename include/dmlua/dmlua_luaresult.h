
// Copyright (c) 2018 brinkqiang
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
