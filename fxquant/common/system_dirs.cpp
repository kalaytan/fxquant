#include "common.h"
#include "system_dirs.h"

#if defined(GNUC_ANY_TARGET)
#include <sys/types.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#elif defined(MSVC_ANY_TARGET)
#include <windows.h>
#include <shlwapi.h>
#include <shlobj.h>
#pragma comment(lib,"shlwapi.lib")
#else
#error Unsupported platform
#endif

namespace fx {
namespace system {

#if defined(GNUC_ANY_TARGET)

bool get_temp_dir(path_type& path)
{
    path = "/tmp";
    return fs::exists(path) && fs::is_directory(path);
}

bool get_exe_path(path_type& path)
{
    bool success = false;
    
    try
    {
        pid_t pid = getpid();

        char buf[PATH_MAX + 1] = { 0 };
        int rc = snprintf(buf, sizeof(buf), "/proc/%u/exe",
            static_cast<unsigned>(pid));

        if (rc > 0)
        {
            char resolved_path[PATH_MAX + 1] = { 0 };

            if (realpath(buf, resolved_path) != NULL)
            {
                path = resolved_path;
                success = fs::exists(path);
            }
        }
    }
    catch (...)
    {
    }
    
    return success;
}

#elif defined(_WIN32)

bool get_temp_dir(path_type& path)
{
    bool success = false;

    try
    {
        path.clear();
        std::wstring wpath(2 * MAX_PATH, wchar_t(0));
        DWORD size = static_cast<DWORD>(wpath.size());
        DWORD ret = GetTempPathW(size, &wpath[0]);

        if ((ret > 0) && (ret < size))
        {
            wpath.resize(static_cast<size_t>(ret));
            path = wpath;
            success = fs::exists(path) && fs::is_directory(path);
        }
    }
    catch (...)
    {
    }

    return success;
}

bool get_exe_path(path_type& path)
{
    bool success = false;

    try
    {
        path.clear();
        std::wstring wpath(2 * MAX_PATH, wchar_t(0));
        DWORD size = static_cast<DWORD>(wpath.size());
        DWORD ret = GetModuleFileName(NULL, &wpath[0], size);

        if ((ret > 0) && (ret < size))
        {
            wpath.resize(static_cast<size_t>(ret));
            path = wpath;
            success = fs::exists(path);
        }
    }
    catch (...)
    {
    }

    return success;
}

bool get_appdata_dir(path_type& path)
{
    path.clear();
    wchar_t buf[MAX_PATH + 1] = { 0 };
    bool ok = (SHGetFolderPathW(NULL, CSIDL_COMMON_APPDATA, NULL, 0, buf) == S_OK);

    if (ok)
    {
        std::wstring wpath;
        wpath.assign(buf);
        path = wpath;
        ok = fs::exists(path) && fs::is_directory(path);
    }

    return ok;
}
#endif

} // namespace system
} // namespace fx
