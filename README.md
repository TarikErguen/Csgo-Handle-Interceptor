# Csgo-Handle-Interceptor

# Was ist Csgo-Handle-Interceptor?
Das Valve Anti Cheat (VAC) System versucht fremde Zugänge zu CS:GO durch externe Prozesse zu verhinden.
Aus diesem Grund gehen sie alle Prozesse durch, die einen Handle zu CS:GO verwenden. 
Ähnlich wie Anti Malware Software wird für jeden Prozess eine Signatur (eine Art Hash) generiert und mit den Einträgen in ihrer Datenbank abgeglichen.
Wurde eine unerlaubte Software verwendet, wird dieser Spieler dauerhaft gesperrt. 

# Wie funktioniert der Interceptor?
Steam startet den Process CSGO.exe, indem es die Windows API CreateProcessW verwendet. Auf msdn kann man sich nähere Informationen zur Funktion einholen:
https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw

Der Prototyp von CreateProcessW sieht wie folgend aus:
```
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
 ```
