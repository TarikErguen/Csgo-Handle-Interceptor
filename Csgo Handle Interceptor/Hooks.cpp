#include "Hooks.h"
#include "MinHook/MinHook.h"
#include <iostream>

// Originaler CSGO Handle wird hier abgespeichert
HANDLE	csgo_handle		= nullptr;

// CreateProcessW Prototyp
using createprocessw_ = decltype(&::CreateProcessW);
createprocessw_ original;

// CreateProcessW wird manipuliert
BOOL WINAPI  CreateProcessWHook(LPCWSTR lpApplicationName,
	LPWSTR                lpCommandLine,
	LPSECURITY_ATTRIBUTES lpProcessAttributes,
	LPSECURITY_ATTRIBUTES lpThreadAttributes,
	BOOL                  bInheritHandles,
	DWORD                 dwCreationFlags,
	LPVOID                lpEnvironment,
	LPCWSTR               lpCurrentDirectory,
	LPSTARTUPINFOW        lpStartupInfo,
	LPPROCESS_INFORMATION lpProcessInformation)
{
	// Originale CreateProcessW Funktion wird aufgerufen.
	auto ret = original(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);
	
	// Einige interessante Parameter werden angezeigt.
	printf("lpApplicationName:%ws\n", lpApplicationName);
	printf("hProcess:%p\n", lpProcessInformation->hProcess);
	printf("hThread:%p\n", lpProcessInformation->hThread);
	printf("dwProcessId:%d\n", lpProcessInformation->dwProcessId);
	printf("hThread:%p\n", lpProcessInformation->hThread);

	// Überprüfe, ob csgo geöffnet wird, falls ja speicher den Handle
	if (wcsstr(lpApplicationName, L"csgo")) {
		csgo_handle		= lpProcessInformation->hProcess;	
		
		// Hook kann entfernt werden. Funktion funktioniert jetzt normal.
		MH_RemoveHook(&CreateProcessW);
	}
	
	// Originaler Rückgabewert
	return ret;
}

// CloseHandle Prototyp
using closehandle_ = decltype(&::CloseHandle);
closehandle_ original_closehandle;

// CloseHandle wird manipuliert
BOOL CloseHandleHook(HANDLE hObject)
{
	// Falls Steam versucht den CSGO Handle zu schließen, verhindere es.
	if (hObject == csgo_handle)
		return 0;

	// Originaler CloseHandle wird abgerufen
	return original_closehandle(hObject);
}

// Hooks werden initialisiert
void Hooks::Run()
{
	// MinHook API's
	MH_Initialize(); 
	MH_CreateHook(&CreateProcessW, CreateProcessWHook, (LPVOID*)&original);
	MH_CreateHook(&CloseHandle, CloseHandleHook, (LPVOID*)&original_closehandle);
	
	MH_EnableHook(MH_ALL_HOOKS);	
}