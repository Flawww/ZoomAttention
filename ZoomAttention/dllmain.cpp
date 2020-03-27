#include "pch.h"
#include "attention_spoofer.h"

void initialize(void* module) {
    AllocConsole();
    freopen_s(reinterpret_cast<FILE**>(stdin), "CONIN$", "r", stdin);
    freopen_s(reinterpret_cast<FILE**>(stdout), "CONOUT$", "w", stdout);

    printf("Initialzing spoofer. Press \"END\" at any time to unspoof and uninject\n");

    g_spoofer = new attention_spoofer();
    g_spoofer->init_hooks();

    while (!(GetAsyncKeyState(VK_END) & 0x8000))
        Sleep(20);

    // perform cleanup
    g_spoofer->destroy_hooks();
    delete g_spoofer;

    fclose(stdin);
    fclose(stdout);
    FreeConsole();

    FreeLibraryAndExitThread(HMODULE(module), 0);
}

int __stdcall DllMain(void* module, uint32_t  reason, void* reserved)
{
    if (reason == DLL_PROCESS_ATTACH) {
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)initialize, module, 0, nullptr);
    }
    return 1;
}

