
#ifndef __SAFESINGLETON_H_INCLUDE__
#define __SAFESINGLETON_H_INCLUDE__

#include <assert.h>

#include <vector>
#include <algorithm>

class ISafeSingleton
{
public:
    virtual ~ISafeSingleton() = 0;
    virtual bool Init() = 0;
    virtual bool UnInit() = 0;
    virtual void Release() = 0;
};

inline ISafeSingleton::~ISafeSingleton(){}

template<typename T>
class HSafeSingleton : public ISafeSingleton
{
public:
    typedef T  SingletonObj;

    HSafeSingleton(){}
    virtual ~HSafeSingleton(){}

public:
    static bool Create()
    {
        if (NULL == m_poInstance)
        {
            m_poInstance = new SingletonObj();
        }
        return NULL != m_poInstance;
    }

    static void Destroy()
    {
        delete m_poInstance;
        m_poInstance = NULL;
    }

    static T* Instance(){ return m_poInstance; }

    virtual bool Init(){ return true; }
    virtual bool UnInit(){ return true; }

    virtual void Release(){ Destroy();}
private:
    static SingletonObj* m_poInstance;
};

template<typename T>
T* HSafeSingleton<T>::m_poInstance = NULL;

class HSafeSingletonFrame
{
public:
    typedef std::vector<ISafeSingleton*> VecSafeSafeSingleton;
    typedef VecSafeSafeSingleton::iterator VecSafeSafeSingletonIt;
    typedef VecSafeSafeSingleton::reverse_iterator VecSafeSafeSingletonRIt;
public:
    HSafeSingletonFrame(){}
    virtual ~HSafeSingletonFrame()
    {
        Release();
    }

    template<typename T>
    void AddSingleton()
    {
        if (!T::Create())
        {
            assert(0);
            return;
        }

        if (std::count(m_vecList.begin(), m_vecList.end(), T::Instance()))
        {
            assert(0);
            return;
        }
        m_vecList.push_back(T::Instance());
    }

    static HSafeSingletonFrame* Instance(){ static HSafeSingletonFrame s_oFrame; return &s_oFrame; }

    void Init()
    {
        for (VecSafeSafeSingletonIt It = m_vecList.begin(); It != m_vecList.end(); ++It)
        {
            if (!(*It)->Init())
            {
                assert(0);
            }
        }
    }

    void UnInit()
    {
        for (VecSafeSafeSingletonRIt It = m_vecList.rbegin(); It != m_vecList.rend(); ++It)
        {
            if (!(*It)->UnInit())
            {
                assert(0);
            }
        }
    }

    void Release()
    {
        for (VecSafeSafeSingletonRIt It = m_vecList.rbegin(); It != m_vecList.rend(); ++It)
        {
            (*It)->Release();
        }

        m_vecList.clear();
    }

private:
    std::vector<ISafeSingleton*> m_vecList;
};

#endif // __SAFESINGLETON_H_INCLUDE__
