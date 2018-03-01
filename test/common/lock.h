
#ifndef __LOCK_H_INCLUDE__
#define __LOCK_H_INCLUDE__

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <pthread.h>
#endif

class CLockNull
{
public:
    CLockNull(){}
    ~CLockNull(){}
    inline void Lock(void){}
    inline void UnLock(void){}
};

class CLock
{
private:
    volatile bool m_bIsSafe;
    inline void SetIsSafe() { m_bIsSafe = true; }
#ifdef WIN32
    CRITICAL_SECTION  m_lock;
#else
    pthread_mutex_t m_lock;
#endif
public:
    CLock(void)
    {
#ifdef WIN32
        InitializeCriticalSection(&m_lock);
#else
        pthread_mutex_init(&m_lock,NULL);
#endif
        SetIsSafe();
    }

    ~CLock()
    {
        if (m_bIsSafe)
        {
#ifdef WIN32
            DeleteCriticalSection(&m_lock);
#else
            pthread_mutex_destroy(&m_lock);
#endif
        }
    }

    inline void Lock(void)
    {
        if (m_bIsSafe)
        {
#ifdef WIN32
            EnterCriticalSection(&m_lock);
#else
            pthread_mutex_lock(&m_lock);
#endif
        }
    }

    inline void UnLock(void)
    {
        if (m_bIsSafe)
        {
#ifdef WIN32
            LeaveCriticalSection(&m_lock);
#else
            pthread_mutex_unlock(&m_lock);
#endif
        }
    }

    CLock(CLock*);
    CLock(const CLock&);
    void operator=(const CLock&);
};

template<typename Mutex>
class CLockGuard
{
private:
    Mutex& m_lock;
public:
    explicit CLockGuard(Mutex& lock)
        : m_lock(lock)
    {
        m_lock.Lock();
    }
    ~CLockGuard()
    {
        m_lock.UnLock();
    }
};

#endif // __LOCK_H_INCLUDE__

