
#ifndef __DMLUA_PARSER_H_INCLUDE__
#define __DMLUA_PARSER_H_INCLUDE__

#ifdef WIN32

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <climits>

#include <errno.h>
#include <direct.h>

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#pragma warning(disable : 4996)
#else

#include <stdio.h>
#include <stdlib.h>

#include <string>
#include <climits>

#include <sys/types.h>
#include <dirent.h>

#include <unistd.h>

#ifndef MAX_PATH
#define MAX_PATH PATH_MAX
#endif

#endif

typedef struct tagFileAttr
{
    tagFileAttr(){ memset(this, 0,sizeof(*this));}
    int isDir;
}SFileAttr;

class CDirectoryParser
{
public:
    CDirectoryParser()
    {
#ifdef WIN32
        m_fHandle = INVALID_HANDLE_VALUE;
#else
        m_dir = NULL;
        memset(m_curDir, 0, sizeof(m_curDir));
#endif
    }

    ~CDirectoryParser()
    {
        Close();
    }

    bool Open( const char* path )
    {
#ifdef WIN32
        m_szPath = path;
        m_szPath += "\\*";
        m_fHandle = INVALID_HANDLE_VALUE;
#else
        strncpy(m_curDir, path, sizeof(m_curDir));
        m_dir = opendir(path);
        if (m_dir == NULL)
        {
            return false;
        }
#endif
        return true;
    }
    void Close()
    {
#ifdef WIN32
        if (m_fHandle != INVALID_HANDLE_VALUE)
        {
            FindClose(m_fHandle);
            m_fHandle = INVALID_HANDLE_VALUE;
        }
#else
        if (m_dir != NULL)
        {
            closedir(m_dir);
            m_dir = NULL;
        }
#endif
    }

    bool Read( char* name, int buflen, SFileAttr* pAttr)
    {
#ifdef WIN32
        WIN32_FIND_DATAA find_data;
        if(m_fHandle == INVALID_HANDLE_VALUE)
        {
            m_fHandle = FindFirstFileA(m_szPath.c_str(), &find_data);
            if (m_fHandle == INVALID_HANDLE_VALUE)
            {
                return false;
            }
        }
        else
        {
            if (FindNextFileA(m_fHandle, &find_data) == 0)
            {
                return false;
            }
        }

        if (strcmp(find_data.cFileName, ".") == 0 ||
            strcmp(find_data.cFileName, "..") == 0 )
        {
            return Read(name, buflen, pAttr);
        }

        _snprintf(name, buflen, "%s", find_data.cFileName);

        if (pAttr != NULL)
        {
            pAttr->isDir = (find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? 1 : 0;
        }

#else
        if (m_dir == NULL)
        {
            return false;
        }

        struct dirent* pdir = readdir(m_dir);
        if (pdir == NULL)
        {
            return false;
        }

        if (strcmp(pdir->d_name, ".") == 0 ||
            strcmp(pdir->d_name, "..") == 0 )
        {
            return Read(name, buflen, pAttr);
        }

        snprintf(name, buflen, "%s", pdir->d_name);

        if (pAttr != NULL )
        {
            char szFullPath[MAX_PATH] ;
            snprintf(szFullPath, buflen, "%s/%s", m_curDir, pdir->d_name);

            struct stat statbuf;
            if (lstat(szFullPath, &statbuf) < 0 )
            {
                return false;
            }

            pAttr->isDir = S_ISDIR(statbuf.st_mode) ;
        }

#endif
        return true;
    }
private:
#ifdef WIN32
    std::string m_szPath;
    HANDLE m_fHandle;
#else
    DIR * m_dir;
    char m_curDir[MAX_PATH];
#endif
};

#endif // __DMLUA_PARSER_H_INCLUDE__
