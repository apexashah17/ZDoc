/*
 *  Copyright (c) 2010,
 *  Gavriloaie Eugen-Andrei (shiretu@gmail.com)
 *
 *  This file is part of crtmpserver.
 *  crtmpserver is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  crtmpserver is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with crtmpserver.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef DFREEBSD
#ifndef _DFREEBSDPLATFORM_H
#define _DFREEBSDPLATFORM_H

#include "platform/baseplatform.h"

//platform includes
#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include <algorithm>
#include <arpa/inet.h>
#include <assert.h>
#include <cctype>
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <fstream>
#include <glob.h>
#include <iostream>
#include <list>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <string>
#include <sys/event.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
using namespace std;

//platform defines
#define DLLEXP
#define HAS_MMAP 1
#define COLOR_TYPE string
#define FATAL_COLOR "\033[01;31m"
#define ERROR_COLOR "\033[22;31m"
#define WARNING_COLOR "\033[01;33m"
#define INFO_COLOR "\033[22;36m"
#define DEBUG_COLOR "\033[01;37m"
#define FINE_COLOR "\033[22;37m"
#define FINEST_COLOR "\033[22;37m"
#define NORMAL_COLOR "\033[0m"
#define SET_CONSOLE_TEXT_COLOR(color) cout<<color
#define READ_FD read
#define WRITE_FD write
#define LASTSOCKETERROR					errno
#define SOCKERROR_CONNECT_IN_PROGRESS	EINPROGRESS
#define SOCKERROR_SEND_IN_PROGRESS		EAGAIN
#define LIB_HANDLER void *
#define FREE_LIBRARY(libHandler) dlclose((libHandler))
#define LOAD_LIBRARY(file,flags) dlopen((file), (flags))
#define LOAD_LIBRARY_FLAGS RTLD_NOW | RTLD_LOCAL
#define OPEN_LIBRARY_ERROR STR(string(dlerror()))
#define GET_PROC_ADDRESS(libHandler, procName) dlsym((libHandler), (procName))
#define LIBRARY_NAME_PATTERN "lib%s.so"
#define PATH_SEPARATOR '/'
#define CLOSE_SOCKET(fd) close(fd)
#define InitNetworking()
#define HAS_KQUEUE_TIMERS
#define KQUEUE_TIMER_MULTIPLIER 1000
#define SET_UNKNOWN 0
#define SET_READ 1
#define SET_WRITE 2
#define SET_TIMER 3
#define FD_READ_CHUNK 32768
#define FD_WRITE_CHUNK FD_READ_CHUNK
#define RESET_TIMER(timer,sec,usec)
#define MAP_NOCACHE 0
#define MAP_NOEXTEND 0
#define NOTE_USECONDS 0
#define FD_COPY(src,dst) memcpy(dst,src,sizeof(fd_set));
#define SRAND() srand(time(NULL));
#define Timestamp struct tm
#define Timestamp_init {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define CLOCKS_PER_SECOND 1000000L
#define GETCLOCKS(result) \
do { \
    struct timeval ___timer___; \
    gettimeofday(&___timer___,NULL); \
    result=(double)___timer___.tv_sec*(double)CLOCKS_PER_SECOND+(double) ___timer___.tv_usec; \
}while(0);

#define GETNTP(result) \
do { \
	struct timeval tv; \
	gettimeofday(&tv,NULL); \
	result=(((uint64_t)tv.tv_sec + 2208988800U)<<32)|((((uint32_t)tv.tv_usec) << 12) + (((uint32_t)tv.tv_usec) << 8) - ((((uint32_t)tv.tv_usec) * 1825) >> 5)); \
}while (0);

#define GETCUSTOMNTP(result,value) \
do { \
	struct timeval tv; \
	tv.tv_sec=value/CLOCKS_PER_SECOND; \
	tv.tv_usec=value-tv.tv_sec*CLOCKS_PER_SECOND; \
	result=(((uint64_t)tv.tv_sec + 2208988800U)<<32)|((((uint32_t)tv.tv_usec) << 12) + (((uint32_t)tv.tv_usec) << 8) - ((((uint32_t)tv.tv_usec) * 1825) >> 5)); \
}while (0);

class DFreeBSDPlatform
    : public BasePlatform
{
public:
    DFreeBSDPlatform();
    virtual ~DFreeBSDPlatform();
};

typedef void (*SignalFnc)(void);

typedef struct _select_event
{
    uint8_t type;
} select_event;


string format(string fmt, ...);
string vformat(string fmt, va_list args);
void replace(string &target, string search, string replacement);
bool fileExists(string path);
string lowercase(string value);
string uppercase(string value);
string changecase(string &value, bool lowerCase);
string tagToString(uint64_t tag);
bool SetFdNonBlock(int32_t fd);
bool SetFdNoSIGPIPE(int32_t fd);
bool SetFdKeepAlive(int32_t fd);
bool SetFdNoNagle(int32_t fd);
bool SetFdReuseAddress(int32_t fd);
bool SetFdOptions(int32_t fd);
bool DeleteFile(string path);
string GetHostByName(string name);
bool isNumeric(string value);
void split(string str, string separator, vector<string> &result);
uint64_t getTagMask(uint64_t tag);
string generateRandomString(uint32_t length);
void ltrim(string &value);
void rtrim(string &value);
void trim(string &value);
map<string, string> mapping(string str, string separator1, string separator2, bool trimStrings);
void splitFileName(string fileName, string &name, string &extension, char separator = '.');
double GetFileModificationDate(string path);
string normalizePath(string base, string file);
bool ListFolder(string root, string path, vector<string> &result);
bool MoveFile(string src, string dst);
void InstallQuitSignal(SignalFnc pQuitSignalFnc);
void InstallConfRereadSignal(SignalFnc pConfRereadSignalFnc);


#endif /* _DFREEBSDPLATFORM_H */
#endif /* DFREEBSD */

