#include "pch.h"
#include "ThreadWrapper.h"

#ifdef _WIN32
#include <Windows.h>
#endif

void setThreadName(std::thread& t, const std::wstring& name) {
#ifdef _WIN32
    auto handle = (HANDLE)t.native_handle();
    SetThreadDescription(handle, name.c_str());
#endif
}
