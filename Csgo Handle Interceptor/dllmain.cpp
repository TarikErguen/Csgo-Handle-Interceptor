#include <Windows.h>
#include "Hooks.h"
#include <stdio.h>


// Öffne Console und rufe Hooks auf
void start()
{
    FILE* file;
    AllocConsole();
    freopen_s(&file, "CONIN$", "r", stdin);
    freopen_s(&file, "CONOUT$", "w", stdout);

    Hooks::Run();
}

// Erstelle einen Thread, um den Ablauf des Originalen Programms nicht zu unterbrechen.
// Verhindert Dead-Lock
BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)start, hModule, NULL, nullptr);
        break;
    }
    return TRUE;
}

