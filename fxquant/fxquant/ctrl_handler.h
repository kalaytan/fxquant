#pragma once

#if defined(_WIN32)
#include <windows.h>
#else
#include <signal.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "event.h"

class ctrl_handler
{
public:
    ctrl_handler();

    void wait()
    {
        event_.wait();
    }

private:
#if defined(_WIN32)
    static BOOL WINAPI handler(DWORD param);
#else
    static void handler(int param);
#endif

private:
    static fx::event event_;
};