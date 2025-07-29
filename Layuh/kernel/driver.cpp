#include "driver.h"
#include <iostream>
#include <memory>

namespace drv {
    bool eac = false;
    INT32 procid = 0;
    HANDLE ProcHandle = nullptr;

    bool Init() {
        if (procid == 0) return false;
        ProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procid);
        return ProcHandle != nullptr;
    }

    void ReadPhys(PVOID address, PVOID buffer, DWORD size) {
        SIZE_T bytesRead = 0;
        if (!ProcHandle) return;
        ReadProcessMemory(ProcHandle, address, buffer, size, &bytesRead);
    }

    void WritePhys(PVOID address, PVOID buffer, DWORD size) {
        SIZE_T bytesWritten = 0;
        if (!ProcHandle) return;
        WriteProcessMemory(ProcHandle, address, buffer, size, &bytesWritten);
    }

    uintptr_t GetBase() {
        uintptr_t baseAddress = 0;
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procid);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            MODULEENTRY32 me32;
            me32.dwSize = sizeof(MODULEENTRY32);
            if (Module32First(hSnapshot, &me32)) {
                baseAddress = reinterpret_cast<uintptr_t>(me32.modBaseAddr);
            }
            CloseHandle(hSnapshot);
        }
        return baseAddress;
    }

    INT32 FindProcess(LPCTSTR process_name) {
        PROCESSENTRY32 pt;
        HANDLE hsnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        pt.dwSize = sizeof(PROCESSENTRY32);
        if (Process32First(hsnap, &pt)) {
            do {
                if (!lstrcmpi(pt.szExeFile, process_name)) {
                    CloseHandle(hsnap);
                    procid = pt.th32ProcessID;
                    return pt.th32ProcessID;
                }
            } while (Process32Next(hsnap, &pt));
        }
        CloseHandle(hsnap);
        return 0;
    }

    std::uintptr_t get_module(const wchar_t* name) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procid);
        if (hSnapshot == INVALID_HANDLE_VALUE) return 0;
        MODULEENTRY32W me32;
        me32.dwSize = sizeof(MODULEENTRY32W);
        if (Module32FirstW(hSnapshot, &me32)) {
            do {
                if (!_wcsicmp(me32.szModule, name)) {
                    CloseHandle(hSnapshot);
                    return reinterpret_cast<std::uintptr_t>(me32.modBaseAddr);
                }
            } while (Module32NextW(hSnapshot, &me32));
        }
        CloseHandle(hSnapshot);
        return 0;
    }
}
