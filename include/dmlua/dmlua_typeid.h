
#ifndef __DMLUA_TYPEID_H_INCLUDE__
#define __DMLUA_TYPEID_H_INCLUDE__

#ifdef __cplusplus

#include <string>
#include <map>
#include <typeinfo>

//#define Mtolua_typeid(L,TI,T) tolua_typeid_reg<TI>(L,T)

struct SLuaTypeInfo
{
    SLuaTypeInfo()
        : m_bInit(false)
    {

    }
    std::string m_strName;
    bool        m_bInit;
};

class CluaTypeid
{
public:
    static CluaTypeid& Instance(){ static CluaTypeid s; return s;}

    template<typename T>
    inline void type_reg(const char* name)
    {
        name_holder<T>().m_strName = name;
        name_holder<T>().m_bInit = true;
    }

    template<typename T>
    inline const char* get_name()
    {
        return name_holder<T>().m_bInit ? name_holder<T>().m_strName.c_str() : NULL;
    }

private:
    template<typename T>
    inline SLuaTypeInfo& name_holder()
    {
        static SLuaTypeInfo sInfo;
        return sInfo;
    }
};

template<typename T>
inline void Mtolua_typeid(lua_State* tolua_S, const char* name)
{
    CluaTypeid::Instance().type_reg<T>(name);
}
#endif

#endif // __DMLUA_TYPEID_H_INCLUDE__
