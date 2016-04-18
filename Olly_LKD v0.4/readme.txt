Olly_LKD by Blabberer
http://www.woodmann.com/forum/showthread.php?14904-ollydbg-2-x-plugin-OLLY_LKD

A small sample plugin for OllyDBG 2 using WinDBG's dbgeng functions especially local kernel debugging output. The plugin is at alpha - Z stage and uses OllyDBG version 2 plugin kit and is built by Winddk (Windows 7 wdk C:\WinDDK\7600.16385.1)

There is a modification required to plugin.h as follows to avoid crashing due to stack unbalance (the same source compiled with vs 2010 and unmodified plugin.h works ok, it seems the wdk compiler is behaving differently the modification to plugin.h is as follows (added a _cdecl so that stack is cleaned up properly)

 C:\ollydbg2beta\plug201ft\Visual C>fc plugin.h d:\Plugin_Template_For_ODBG_20001_WDK\plugin.h
 Comparing files plugin.h and D:\PLUGIN_TEMPLATE_FOR_ODBG_20001_WDK\PLUGIN.H
 ***** plugin.h

 typedef int MENUFUNC(struct t_table *,wchar_t *,ulong,int);

 ***** D:\PLUGIN_TEMPLATE_FOR_ODBG_20001_WDK\PLUGIN.H

 typedef int _cdecl MENUFUNC(struct t_table *,wchar_t *,ulong,int);

 *****
 C:\ollydbg2beta\plug201ft\Visual C>

The source is gibberish on top of the template I posted earlier for vs2010 at the moment so I am not posting refer to Kayakers blog about ollydb.lib, a compiled binary is attached. Any comments, feedback, sugestions, criticisms are welcome.

 1) To use it copy plugin DLL to ollydbg.exe folder.
 2) Copy the following Windbgs extensions / dlls (6.12 ) to the folder where ollydbg.exe resides; uext, symsrv, ntsdexts, kext, kdexts, exts, ext, dbghelp, dbgeng
 3) Click the menu a getstring dialog will be presented assuming your debugee is msgbox.exe. If you type in "!process 0 0 msgbox.exe " without the quotes you will be presented with the following details:
 
 Log data
Address   Message
          Connected to Windows XP 2600 x86 compatible target at (Thu Sep  6 05:58:23.578 2012 (UTC + 5:30)), ptr64 FALSE
          Symbol search path is:
          SRV*F:\symbols*http://msdl.microsoft.com/download/symbols
          Executable search path is:
          *******************************************************************************
          WARNING: Local kernel debugging requires booting with kernel
          debugging support (/debug or bcdedit -debug on) to work optimally.
          *******************************************************************************
          Windows XP Kernel
          Version 2600
           (Service Pack 3)
           UP
          Free x86 compatible
          Product:
          WinNt
          , suite:
           TerminalServer
           SingleUserTS
          Built by: 2600.xpsp_sp3_gdr.100216-1514
          Machine Name:
          Kernel base = 0x804d7000 PsLoadedModuleList = 0x80554040
          Debug session time: Thu Sep  6 05:58:23.656 2012 (UTC + 5:30)
          System Uptime: 0 days 17:45:57.225
          PROCESS 86ba98e0
            SessionId: 0  Cid: 0ce8    Peb: 7ffd8000  ParentCid: 0894
              DirBase: 0f8c0420  ObjectTable: e2a8ea90  HandleCount:  14.
              Image: msgbox.exe
              VadRoot 85f24388 Vads 36 Clone 0 Private 104. Modified 0. Locked 0.
              DeviceMap e30a2340
              Token                             e172e040
              ElapsedTime                       00:04:07.015
              UserTime                          00:00:00.031
              KernelTime                        00:00:00.000
              QuotaPoolUsage[PagedPool]         26588
              QuotaPoolUsage[NonPagedPool]      1440
              Working Set Sizes (now,min,max)  (583, 50, 345) (2332KB, 200KB, 1380KB)
              PeakWorkingSetSize                583
              VirtualSize                       12 Mb
              PeakVirtualSize                   13 Mb
              PageFaultCount                    609
              MemoryPriority                    BACKGROUND
              BasePriority                      8
              CommitCharge                      124
              DebugPort                         85f25ec0
              Setting context for this process...

                                                 
        THREAD 863f7b08  Cid 0ce8.063c  Teb: 7ffdf000 Win32Thread: e4262e10 WAIT: (Executive) KernelMode Non-Alertable
                      a8eb87d4  SynchronizationEvent
                  Not impersonating
                  DeviceMap                 e30a2340
                  Owning Process            0       Image:         <Unknown>
                  Attached Process          86ba98e0       Image:         msgbox.exe
                  Wait Start TickCount      4077495        Ticks: 15786 (0:00:04:06.656)
                  Context Switch Count      92                 LargeStack
                  UserTime                  00:00:00.015
                  KernelTime                00:00:00.000
          *** WARNING: Unable to verify checksum for C:\Documents and Settings\Admin\My Documents\ollydbg2beta\odbg201ft\msgbox.exe
          *** ERROR: Module load completed but symbols could not be loaded for C:\Documents and Settings\Admin\My Documents\ollydbg2beta\odbg201ft\msgbox.exe
                  Win32 Start Address msgbox (0x00401000)
                  Start Address kernel32!BaseProcessStartThunk (0x7c810705)
                  Stack Init a8eb9000 Current a8eb8758 Base a8eb9000 Limit a8eb5000 Call 0
                  Priority 10 BasePriority 8 PriorityDecrement 0 DecrementCount 0
                  ChildEBP RetAddr
                  a8eb8770 80500cf0 nt!KiSwapContext+0x2e (FPO: [Uses EBP] [0,0,4])
                  a8eb877c 804f9d72 nt!KiSwapThread+0x46 (FPO: [0,0,0])
                  a8eb87a4 80638fc4 nt!KeWaitForSingleObject+0x1c2 (FPO: [Non-Fpo])
                  a8eb8884 8063a099 nt!DbgkpQueueMessage+0x17c (FPO: [Non-Fpo])
                  a8eb88a8 8063a1cb nt!DbgkpSendApiMessage+0x45 (FPO: [Non-Fpo])
                  a8eb8934 804fcb42 nt!DbgkForwardException+0x8f (FPO: [Non-Fpo])
                  a8eb8cf4 8053e0a1 nt!KiDispatchException+0x1f4 (FPO: [Non-Fpo])
                  a8eb8d5c 8053e7b1 nt!CommonDispatchException+0x4d (FPO: [0,20,0])
                  a8eb8d5c 00401001 nt!KiTrap03+0xad (FPO: [0,0] TrapFrame @ a8eb8d64)
          WARNING: Stack unwind information not available. Following frames may be wrong.
                  0013fff0 00000000 msgbox+0x1001