#pragma once
#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <cstdint>

namespace drv {
    extern bool eac;
    extern INT32 procid;
    extern HANDLE ProcHandle;

    bool Init();
    void ReadPhys(PVOID address, PVOID buffer, DWORD size);
    void WritePhys(PVOID address, PVOID buffer, DWORD size);
    uintptr_t GetBase();
    INT32 FindProcess(LPCTSTR process_name);
    std::uintptr_t get_module(const wchar_t* name);
}

template <typename T>
T read(uint64_t address) {
    T buffer{};
    drv::ReadPhys((PVOID)address, &buffer, sizeof(T));
    return buffer;
}

template <typename T>
T write(uint64_t address, T buffer) {
    drv::WritePhys((PVOID)address, &buffer, sizeof(T));
    return buffer;
}
