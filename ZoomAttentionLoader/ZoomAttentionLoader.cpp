#include <Windows.h>
#include <Psapi.h>
#include <iostream>
#include <string>
#include <vector>


int main()
{
    char dll_path[MAX_PATH];
    char window_name[MAX_PATH];
    DWORD process_id = 0;

    GetFullPathNameA("ZoomAttention.dll", MAX_PATH, dll_path, nullptr);
    printf("Trying to load DLL %s\nPlease make the Zoom Meeting the active window...\n", dll_path);

    while (true) {
        auto handle = GetForegroundWindow();
        if (handle) {
            GetWindowTextA(handle, window_name, MAX_PATH);
            GetWindowThreadProcessId(handle, &process_id);
            if (strstr(window_name, "Zoom Meeting"))
                break;
        }

        Sleep(100);
    }

    printf("Found: %s (PID: %i)\nStarting injection\n", window_name, process_id);

    auto proc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, process_id);
    if (!proc) {
        printf("OpenProcess failed with error: %x\n", GetLastError());
        return 0;
    }
    printf("Opened process with %X\n", PROCESS_ALL_ACCESS);

    auto buf = VirtualAllocEx(proc, nullptr, MAX_PATH, MEM_COMMIT, PAGE_READWRITE);
    if (!buf) {
        printf("VirtualAllocEx failed with %X\n", GetLastError());
        return 0;
    }
    printf("Allocated %i bytes at address %X\n", MAX_PATH, buf);

    auto res = WriteProcessMemory(proc, buf, dll_path, MAX_PATH, nullptr);
    if (!res) {
        printf("WriteProcessMemory failed with %X\n", GetLastError());
        return 0;
    }
    printf("Wrote DLL path to allocation\n");

    CreateRemoteThreadEx(proc, nullptr, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, buf, 0, nullptr, nullptr);
    printf("Created thread in remote process at address %X (LoadLibraryA)\nExiting...\n", LoadLibraryA);

    Sleep(5000);

    // Cleanup
    VirtualFreeEx(proc, buf, 0, MEM_RELEASE);

    return 0;
}