#ifndef __STDAFX_H__
#define __STDAFX_H__

#if defined(_WIN32) || defined(WIN32)
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif

    #ifndef NOMINMAX
        #define NOMINMAX
    #endif

    // Критично: winsock2.h должен быть раньше windows.h
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <windows.h>
#endif

#if defined(__linux__) || defined(__APPLE__)
    #define LINUX_OR_MACOS
#endif

#endif // __STDAFX_H__