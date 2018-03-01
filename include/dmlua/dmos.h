
#ifndef __DMOS_H_INCLUDE__
#define __DMOS_H_INCLUDE__

#ifdef WIN32

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <list>

#include <set>
#include <map>

#include <sstream>
#include <fstream>
#include <iostream>

#include <winsock2.h>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <direct.h>
#include <process.h>
#include <conio.h>

#pragma comment(lib, "ws2_32.lib")

namespace stdext
{

}

namespace std
{
    using namespace stdext;
}

#define VSNPRINTF _vsnprintf
#define SleepMs(x) Sleep(x)
#ifndef INFINITE
#define INFINITE 0xffffffff
#endif

#else

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <string>
#include <vector>
#include <queue>
#include <deque>
#include <list>

#include <set>
#include <map>

#include <sstream>
#include <fstream>
#include <iostream>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include <unistd.h>
#include <ncurses.h>
#include <signal.h>
#include <netdb.h>

#ifndef MAX_PATH
#define MAX_PATH    PATH_MAX
#endif
#define VSNPRINTF vsnprintf
#define SleepMs(x) usleep(x*1000)
#ifndef INFINITE
#define INFINITE 0xffffffff
#endif
#endif

#define PATH_IS_DELIMITER(x)  ('\\' == x || '/' == x)

#ifdef WIN32
#define PATH_DELIMITER '\\'
#else
#define PATH_DELIMITER '/'
#endif

#define DMASSERT assert

#endif // __DMOS_H_INCLUDE__
