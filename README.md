# Csgo-Handle-Interceptor


# Credits
MinHook - https://www.codeproject.com/Articles/44326/MinHook-The-Minimalistic-x-x-API-Hooking-Libra
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
![alt text](https://github.com/TarikErguen/Csgo-Handle-Interceptor/blob/master/Vorher.PNG)

Nachher:
![alt text](https://github.com/TarikErguen/Csgo-Handle-Interceptor/blob/master/nachher.PNG)

Der erste byte wird mit dem Opcode E9, also jmp in asm x86 überschrieben.
Die nächsten 4 bytes beinhalten die relative Adresse zu unserer modifizierten Funktion CreateProcessWHook

Das bedeutet nun: Immer wenn Steam CreateProcessW aufruft, wird zunächst unsere Funktion CreateProcessWHook aufgerufen.

Wichtig: Der Funktionsprototyp des Hooks muss identisch mit der Originalen Funktion sein, ansonsten entstehen Fehler auf dem Stack und das Programm stürzt ab.

Wo werden Hooks sonst noch verwendet?
Overlays (z.B. für UI), Sicherheitsdienste, Malware, Input (Maus, Keyboard)

# Wie bekommen wir Zugang zum Handle?
Bereits oben habe ich erklärt, dass wir den Wert von hProcess (welcher ein Pointer ist) auslesen möchten. Dafür brauchen wir die Adresse vom Pointer der auf diese Variable zeigt, also lpProcessInformation.
Unser Hook muss also folgendermaßen aussehen:

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
{
	// Originale CreateProcessW Funktion wird aufgerufen.
	auto ret = original(lpApplicationName, lpCommandLine, lpProcessAttributes, lpThreadAttributes, bInheritHandles, dwCreationFlags, lpEnvironment, lpCurrentDirectory, lpStartupInfo, lpProcessInformation);	
	
	// Überprüfe, ob csgo geöffnet wird, falls ja speicher den Handle
	if (wcsstr(lpApplicationName, L"csgo")) {
		csgo_handle		= lpProcessInformation->hProcess;	
		
		// Hook kann entfernt werden. Funktion funktioniert jetzt normal.
		MH_RemoveHook(&CreateProcessW);
	}
	
	// Originaler Rückgabewert
	return ret;
}
 ```
Damit der Prozess überhaupt gestartet werden kann, muss logischerweise die originale CreateProcessW noch einmal von uns aufgerufen werden.
Dabei dürfen wir jedoch nicht die Originale Funktionsadresse verwenden, da wir ja sonst immer wieder unseren Hook aufrufen würden.
Wir rufen CreateProcessW auf, indem wir 6 bytes (da wir ja 5 bytes überschrieben hatten) überspringen. 

Anschließend überprüfen wir, ob Steam csgo öffnen möchte mit einem einfachen String-Vergleich. Anschließend wird der Handle aus hProcess gespeichert und der Hook kann wieder entfernt werden.

Doch es gibt ein Problem...

Nach einer Weile ist der Handle ungültig. Beim Debuggen von Steam ist mir aufgefallen, dass der Handle kurze Zeit nachdem er kreiert wurde, mittels CloseHandle geschlossen wird. Also muss ein Weg her, um dies zu vermeiden.

Dafür hooke ich auch noch CloseHandle und verhindere, dass die originale Funktion aufgerufen wird, sodass der Handle nicht mehr von Steam geschlossen werden kann.

 ```
// CloseHandle wird manipuliert
BOOL CloseHandleHook(HANDLE hObject)
{
	// Falls Steam versucht den CSGO Handle zu schließen, verhindere es.
	if (hObject == csgo_handle)
		return 0;

	// Originaler CloseHandle wird abgerufen
	return original_closehandle(hObject);
}
 ```
