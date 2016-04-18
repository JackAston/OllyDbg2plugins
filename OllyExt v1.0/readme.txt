OllyExt is a plugin for Olly 2.xx debugger.

The main intention of this plugin is to provide the biggest anti-anti debugging features
and bugfixes for Olly 2.xx. Updates will come... :)

The currently supported protections are the following:
	- IsDebuggerPresent
	- NtGlobalFlag
	- HeapFlag
	- ForceFlag
	- CheckRemoteDebuggerPresent
	- OutputDebugString
	- CloseHandle
	- SeDebugPrivilege
	- BlockInput
	- ProcessDebugFlags
	- ProcessDebugObjectHandle
	- TerminateProcess
	- NtSetInformationThread
	- NtQueryObject
	- FindWindow
	- NtOpenProcess
	- Process32First
	- Process32Next
	- ParentProcess
	- GetTickCount
	- timeGetTime
	- QueryPerformanceCounter
	- ZwGetContextThread
	- NtSetContextThread
	- KdDebuggerNotPresent
	- KdDebuggerEnabled
	- NtSetDebugFilterState
	- ProtectDRX
	- HideDRX

The currently supported bugfixes are the following:
	- Caption change
	- Kill Anti-Attach ( dll integrity check )

Requirements:
 - Microsoft Visual C++ 2010 Redistributable Package (x86)

OS support:
 - WinXP x32
 - WinXP WoW64
 - Win7 x32
 - Win7 WoW64

If you have any problem just notify me.

About the author:

Created by Ferrit
Send your bugreports/comments to ferrit.rce@gmail.com

Enjoy :P
