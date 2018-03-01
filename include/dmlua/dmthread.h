
#ifndef __DMTHREAD_H_INCLUDE__
#define __DMTHREAD_H_INCLUDE__

#include "dmos.h"

class IThread
{
public:
    virtual ~IThread(){}
    virtual void ThrdProc() = 0;
    virtual void Terminate() = 0;
};

class IThreadCtrl
{
public:
    virtual ~IThreadCtrl(){}
    virtual void Resume() = 0;
    virtual void Suspend() = 0;
    virtual void Stop() = 0;
    virtual bool Kill(unsigned int dwExitCode) = 0;
    virtual bool WaitFor(unsigned int dwWaitTime = -1) = 0;
    virtual unsigned int GetThreadID() = 0;
    virtual IThread* GetThread() = 0;
    virtual void Release() = 0;
};

class CThreadCtrl : public IThreadCtrl
{
public:
    CThreadCtrl()
    {
#ifdef WIN32
        m_bIsStop       = true;
        m_bNeedWaitFor  = true;
        m_hThread       = INVALID_HANDLE_VALUE;
        m_dwThreadID    = -1;
        m_poThread      = NULL;
#else
        m_bIsStop       = true;
        m_bNeedWaitFor  = true;
        m_ID            = 0;
        m_poThread      = NULL;
#endif
    }

    virtual ~CThreadCtrl()
    {

    }

public:
    virtual void Resume(void)
    {
#ifdef WIN32
        ResumeThread(m_hThread);
#else
		DMASSERT(0);
#endif
    }

    virtual void Suspend()
    {
#ifdef WIN32
        SuspendThread(m_hThread);
#else
		DMASSERT(0);
#endif
    }

    virtual void Stop(void)
    {
#ifdef WIN32
        m_poThread->Terminate();
#else
        m_poThread->Terminate();
#endif
    }

    virtual bool Kill(unsigned int dwExitCode)
    {
#ifdef WIN32
        if (INVALID_HANDLE_VALUE == m_hThread)
        {
            return false;
        }

        if (!TerminateThread(m_hThread, dwExitCode))
        {
            return false;
        }

        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
        return true;
#else
        pthread_cancel(m_ID);
        return true;
#endif
    }

    virtual bool WaitFor(unsigned int dwWaitTime = INFINITE)
    {
#ifdef WIN32
        if (!m_bNeedWaitFor || INVALID_HANDLE_VALUE == m_hThread)
        {
            return false;
        }

        unsigned int dwRet = WaitForSingleObject(m_hThread, dwWaitTime);
        CloseHandle(m_hThread);
        m_hThread = INVALID_HANDLE_VALUE;
        m_bIsStop = true;

        if(WAIT_OBJECT_0 == dwRet)
          return true;

        return false;
#else
        if(false == m_bNeedWaitFor)
          return false;

        m_bIsStop = true;
        pthread_join(m_ID, NULL);
        m_ID = -1;
        return true;
#endif
    }

    virtual void Release(void)
    {
        delete this;
    }

    virtual unsigned int GetThreadID(void)
    {
#ifdef WIN32
        return m_dwThreadID;
#else
        return (unsigned int)m_ID;
#endif

    }

    virtual IThread* GetThread(void)
    {
        return m_poThread;
    }

#ifdef WIN32
    static unsigned int __stdcall StaticThreadFunc(void *arg)
#else
        static void* StaticThreadFunc(void *arg)
#endif
        {
#ifdef WIN32
            CThreadCtrl *poCtrl = (CThreadCtrl *)arg;
            poCtrl->m_bIsStop = false;
            poCtrl->m_poThread->ThrdProc();

            return 0;
#else
            CThreadCtrl* poCtrl = (CThreadCtrl *)arg;
            pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, 0);
            pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);

            sigset_t new_set,old_set;
            sigemptyset(&new_set);
            sigemptyset(&old_set);
            sigaddset(&new_set, SIGHUP);
            sigaddset(&new_set, SIGINT);
            sigaddset(&new_set, SIGQUIT);
            sigaddset(&new_set, SIGTERM);
            sigaddset(&new_set, SIGUSR1);
            sigaddset(&new_set, SIGUSR2);
            sigaddset(&new_set, SIGPIPE);
            pthread_sigmask(SIG_BLOCK, &new_set, &old_set);

            if(!poCtrl->m_bNeedWaitFor)
            {
                pthread_detach(pthread_self());
            }

            poCtrl->m_bIsStop = false;
            poCtrl->m_poThread->ThrdProc();

            return NULL;
#endif
        }

    bool Start(IThread* poThread, bool bNeedWaitFor = true, bool bSuspend = false)
    {
#ifdef WIN32
        m_bNeedWaitFor = bNeedWaitFor;
        m_poThread = poThread;

        if (bSuspend)
        {
            m_hThread = (HANDLE)_beginthreadex(0, 0, StaticThreadFunc, this, CREATE_SUSPENDED, &m_dwThreadID);
        }
        else
        {
            m_hThread = (HANDLE)_beginthreadex(0, 0, StaticThreadFunc, this, 0, &m_dwThreadID);
        }

        if (INVALID_HANDLE_VALUE == m_hThread)
        {
            return false;
        }

        return true;
#else
        m_bNeedWaitFor = bNeedWaitFor;
        m_poThread = poThread;

        if (0 != pthread_create(&m_ID, NULL, (void *(*)(void *))StaticThreadFunc, this))
        {
            return false;
        }

        return true;
#endif
    }

protected:
#ifdef WIN32
    volatile bool   m_bIsStop;
    bool            m_bNeedWaitFor;
    HANDLE          m_hThread;
    unsigned int    m_dwThreadID;
    IThread*        m_poThread;
#else
    volatile bool   m_bIsStop;
    pthread_t       m_ID;
    IThread*        m_poThread;
    bool            m_bNeedWaitFor;
#endif
};

inline IThreadCtrl* CreateThreadCtrl()
{
    return new CThreadCtrl;
}

#endif // __DMTHREAD_H_INCLUDE__