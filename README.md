# Csgo-Handle-Interceptor

# Was ist Csgo-Handle-Interceptor?
Das Valve Anti Cheat (VAC) System versucht fremde Zugänge zu CS:GO durch externe Prozesse zu verhinden.
Aus diesem Grund gehen sie alle Prozesse durch, die einen Handle zu CS:GO verwenden. 
Ähnlich wie Anti Malware Software wird für jeden Prozess eine Signatur (eine Art Hash) generiert und mit den Einträgen in ihrer Datenbank abgeglichen.
Wurde eine unerlaubte Software verwendet, wird dieser Spieler dauerhaft gesperrt. 

# Was soll erreicht werden?
Steam startet den Process CSGO.exe, indem es die Windows API CreateProcessW verwendet. Auf msdn kann man sich nähere Informationen zur Funktion einholen:
https://docs.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-createprocessw

Der Prototyp von CreateProcessW sieht wie folgend aus:
```
BOOL WINAPI  CreateProcessW(LPCWSTR lpApplicationName,
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
Wird also ein Prozess gestartet, wird der Handle und weitere Variablen im Struct PROCESS_INFORMATION gespeichert. Über den Pointer lpProcessInformation kann man auf die Member dieses Structs zugreifen.
PROCESS_INFORMATION sieht dabei wie folgt aus:
 ```
typedef struct _PROCESS_INFORMATION {
    HANDLE hProcess;
    HANDLE hThread;
    DWORD dwProcessId;
    DWORD dwThreadId;
} PROCESS_INFORMATION, *PPROCESS_INFORMATION, *LPPROCESS_INFORMATION;
 ```
hProcess speichert den Handle zum Prozess, den wir auslesen müssen.
Nun gibt es aber ein Problem. CreateProcessW wird von Steam gestartet und nicht von uns. Wir wissen nicht wann und wie er gestartet wird. Dafür setzen wir einen sogenannten Hook auf diese Funktion.
 
 # Was ist ein Hook?
Ein Hook ist ein Weg den ursprünglichen Ablauf einer Funktion zu modifizieren. Es gibt viele Wege eine Funktion zu hooken. In diesem Projekt wird das sogenannte Jump/Detour/Trampoline-Hook praktiziert.

Die ersten 5 bytes von CreateProcessW werden überschrieben.

Vorher:

