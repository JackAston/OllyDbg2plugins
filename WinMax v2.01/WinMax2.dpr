//====================================================================================================================//
//
//  Window Maximizer v2.00 -> Plugin for OllyDBG v2.xx
//  By BoB -> BobSoft
//
//====================================================================================================================//

Library WinMax2;

{$IMAGEBASE $04000000}

Uses
  Windows;


//====================================================================================================================//
// Plugin constants ..

Const
  Plugin_Name = 'Window Maximizer';
  Plugin_Vers = 'v2.01';
  Plugin_Auth = 'BoB / BobSoft';
  Plugin_Site = 'http://bob.droppages.com/';
  Plugin_Help = 'This plugin was coded by '+Plugin_Auth+#10#10+Plugin_Site;


//====================================================================================================================//
// Common type definition ..

Type
  Int32   = LongInt;
  UInt32  = DWord;
  PUInt32 = ^UInt32;


//====================================================================================================================//
// PDK Constants ..

Const
  OLLY_VERSION = 201;
  PLUGIN_VERSION = $20003;
  PWM_MAIN: PWideChar = 'MAIN';


//====================================================================================================================//
// Menu structure definition ..

Type
  MENUFUNC = Function (Const Table: Pointer; Const Text: PWideChar; Const Index: UInt32; Const Mode: Int32): Int32; Cdecl;
  PMenu = ^TMenu;
  TMenu = Record                    // Menu descriptor
    name: PWideChar;                // Menu command
    help: PWideChar;                // Explanation of command
    shortcutid: Int32;              // Shortcut identifier, K_xxx
    menucmd: MENUFUNC;              // Function that executes menu command
    submenu: PMenu;                 // Pointer to descriptor of popup menu
    Case Boolean Of
      True:  (Index: UInt32);       // Argument passed to menu function
      False: (hSubMenu: hMenu);     // Handle of pulldown menu
  End;


//====================================================================================================================//
// Menu constants ..

Const
  // Input modes of menu functions.
  MENU_VERIFY    = 0;               // Check if menu item applies
  MENU_EXECUTE   = 1;               // Execute menu item
  // Values returned by menu functions on MENU_VERIFY.
  MENU_ABSENT    = 0;               // Item doesn't appear in menu
  MENU_NORMAL    = 1;               // Ordinary menu item
  MENU_CHECKED   = 2;               // Checked menu item
  MENU_CHKPARENT = 3;               // Checked menu item + checked parent
  MENU_GRAYED    = 4;               // Inactive menu item
  MENU_SHORTCUT  = 5;               // Shortcut only, not in menu
  // Values returned by menu functions on MENU_EXECUTE.
  MENU_NOREDRAW  = 0;               // Do not redraw owning window
  MENU_REDRAW    = 1;               // Redraw owning window


//====================================================================================================================//
// Menu settings ..

Function  MainMenu_Func(Const Table: Pointer; Const Text: PWideChar; Const Index: UInt32; Const Mode: Int32): Int32; Cdecl; forward;
Const
  MainMenu: Array [0 .. 2] Of TMenu = (
    (name: 'Active'; help: Nil; shortcutid: 0; menucmd: MainMenu_Func; submenu: Nil; Index: 1),
    (name: '|About'; help: Nil; shortcutid: 0; menucmd: MainMenu_Func; submenu: Nil; Index: 2),
    (name: Nil;      help: Nil; shortcutid: 0; menucmd: Nil;           submenu: Nil; Index: 0)
  );


//====================================================================================================================//
// Global variables ..

Var
  PluginActive: Boolean = True;
  OllyDbgBase:  UInt32  = 0;
  hwollymain:   PUint32 = Nil;
  hwclient:     PUInt32 = Nil;
  Addtolist:    Procedure (Const Addr: UInt32; Const Colour: Int32; Const Format: PWideChar); Cdecl Varargs;
  Getfromini:   Function  (Const Filename, Section, Key, Format: PWideChar): Int32; Cdecl Varargs;
  Writetoini:   Function  (Const Filename, Section, Key, Format: PWideChar): Int32; Cdecl Varargs;
  Suspendallthreads: Procedure; StdCall;
  Resumeallthreads:  Procedure; StdCall;


//====================================================================================================================//
// Return length of string ..

Function  StrLen(Const Str: PAnsiChar): UInt32; Assembler;
Asm
  Or      EAX, EAX    // EAX = Param ..
  Je      @Exit
  Push    EDI
  Push    ECX
  XChg    EDI, EAX    // EDI = Str ..
  Xor     EAX, EAX
  Xor     ECX, ECX
  Dec     ECX
  RepNE   ScasB
  Dec     EAX
  Dec     EAX
  Sub     EAX, ECX
  Pop     ECX
  Pop     EDI
@Exit:
End;


//====================================================================================================================//
// Return minimum value of two params ..

Function  Min(Const A, B: Int32): Int32;
Begin
  If (A <= B) Then Result := A Else Result := B;
End;


//====================================================================================================================//
// AllocMem - Allocate memory ..

Function  AllocMem(Const Size: UInt32): Pointer;
Begin
  System.GetMem(Result, Size);
  If (Result <> Nil) Then FillChar(Result^, Size, 0);
End;


//====================================================================================================================//
// Do the work of the plugin ..

Procedure Process;
Const
  WM_MDIGETACTIVE = $0229;
Var
  B: Bool;
  H: HWnd;
Begin
  If PluginActive Then Begin
    // Maximize current MDI window if not already maximized ..
    H := SendMessage(HWnd(HWCLIENT^), WM_MDIGETACTIVE, 0, Integer(@B));
    If (Not B) And (H > 0) Then ShowWindow(H, SW_SHOWMAXIMIZED);
  End;
End;


//====================================================================================================================//
// Write MaxLen chars from ansi string as unicode to buffer ..  Returns length ..

Function  WriteUnicode(Const Src: PAnsiChar; Const Dst: PWideChar; Const MaxLen: UInt32) : UInt32;
Var
  Len: UInt32;
Begin
  Result := 0;
  If IsBadWritePtr(Dst, Max_Path) Then Exit;
  FillMemory(Dst, MaxLen Shl 1, 0);
  If IsBadReadPtr(Src, Max_Path) Then Exit;
  Result := StrLen(Src);
  If (Result = 0) Then Exit;
  Len := Min(Result, MaxLen);

  // Get Length of buffer needed ..
  Result := MultiByteToWideChar(GetACP, 0, Src, Len, Dst, 0);
  // Convert string and return length ..
  Result := MultiByteToWideChar(GetACP, 0, Src, Len, Dst, Result);
End;


//====================================================================================================================//
// Return allocated ansi string converted from widechar ..

Function  AsAnsi(Const Src: PWideChar) : PAnsiChar;
Var
  WL:  UInt32;
  Len: UInt32;
Begin
  Result := Nil;
  If IsBadReadPtr(Src, Max_Path) Then Exit;

  // Get length of Wide string ..
  WL := 0;
  While (Src[WL] <> #0) Do Inc(WL);

  // Get Length of buffer needed ..
  Len := WideCharToMultiByte(GetACP, 0, Src, WL, Result, 0, Nil, Nil);
  Result := AllocMem(Len);
  // Convert string and return ..
  WideCharToMultiByte(GetACP, 0, Src, WL, Result, Len, Nil, Nil);
End;


//====================================================================================================================//
// Return string from widechar ..

Function  WideToStr(Const Src: PWideChar): String;
Var
  A: PAnsiChar;
  L: UInt32;
Begin
  Result := '';
  A := AsAnsi(Src);
  If (Not IsBadReadPtr(A, Max_Path)) Then Try
    L := StrLen(A);
    SetLength(Result, L);
    Move(A^, Result[1], L);
  Finally
    FreeMem(A);
  End;
End;


//====================================================================================================================//
// Callback function to handle menu ..

Function  MainMenu_Func(Const Table: Pointer; Const Text: PWideChar; Const Index: UInt32; Const Mode: Int32): Int32; Cdecl;
Begin
  Case Mode Of
    MENU_VERIFY:  Begin
      Result := MENU_NORMAL;
      If (Index = 1) And PluginActive Then Result := MENU_CHECKED;
    End;
    MENU_EXECUTE: Begin
      Result := MENU_NOREDRAW;
      Case Index Of
        1 : Begin
          PluginActive := Not PluginActive;
          If PluginActive And (hwollymain^ <> 0) Then Begin
            ShowWindow(hwollymain^, SW_SHOWMAXIMIZED);
            Process;
          End;
        End;
        2 : Begin  // About window ..
          If (@Resumeallthreads <> Nil) Then Resumeallthreads;
          Try
            If (Not IsBadReadPtr(hwollymain, 4)) And (hwollymain^ <> 0) Then Begin
              MessageBox(hwollymain^, Plugin_Help, 'About', MB_ICONINFORMATION);
            End;
          Finally
            If (@Suspendallthreads <> Nil) Then Suspendallthreads;
          End;
        End;
      End;
    End;
  Else
    Result := 0;
  End;
End;


//====================================================================================================================//
// Set PluginName, PluginVersion to be unicode strings of MAX 30 chars ..

Function  ODBG2_Pluginquery(Const ollydbgversion: Int32; Const features: PUInt32; Const pluginname{SHORTNAME}, pluginversion{SHORTNAME}: PWideChar) : Int32; Cdecl;
Begin
  Result := 0;  // Return null if fail, ODBG2_Plugindestroy is called before freeing library ..

  // Fill needed string buffers ..
  WriteUnicode(Plugin_Name, PluginName,    30);
  WriteUnicode(Plugin_Vers, PluginVersion, 30);

  // Return version of PDK used to build plugin ..  I still think this constant should be called PDK_VERSION!
  If (ollydbgversion >= OLLY_VERSION) Then Result := PLUGIN_VERSION;
End;


//====================================================================================================================//
// Init function, called once only!

Function  ODBG2_Plugininit: Int32; Cdecl;
Begin
  Result := -1;

  // Get used exports from OllyDbg ..
  OllyDbgBase := GetModuleHandleA(Nil);
  Suspendallthreads := GetProcAddress(OllyDbgBase, 'Suspendallthreads');
  Resumeallthreads  := GetProcAddress(OllyDbgBase, 'Resumeallthreads');
  Writetoini        := GetProcAddress(OllyDbgBase, 'Writetoini');
  Getfromini        := GetProcAddress(OllyDbgBase, 'Getfromini');
  Addtolist         := GetProcAddress(OllyDbgBase, 'Addtolist');
  If (@Addtolist = Nil) Then Exit;

  // Get addresses of variables ..
  hwollymain := GetProcAddress(OllyDbgBase, '_hwollymain');  // Handle of Olly window ..
  hwclient   := GetProcAddress(OllyDbgBase, '_hwclient');    // Handle of MDI window ..

  // Output to log window ..
  Addtolist(0, 1, Plugin_Name+' '+Plugin_Vers);
  Addtolist(0, 2, '  By '+Plugin_Auth);

  // Read settings from ini ..
  Getfromini(Nil, Plugin_Name, 'Plugin is active', '%i', @PluginActive);

  // Init ..
  If PluginActive And (hwollymain^ <> 0) Then Begin
    ShowWindow(hwollymain^, SW_SHOWMAXIMIZED);
    Addtolist(0, 0, '  Plugin active, all windows will stay maximized ..');

    // Maximize MDI windows ..
    Process;
  End;
  Addtolist(0, 0, ' ');

  Result := 0;
End;


//====================================================================================================================//
// Return pointer to menu ..

Function  ODBG2_Pluginmenu(Const Menutype: PWideChar): PMenu; Cdecl;
Begin
  Result := Nil;
  If (WideToStr(Menutype) = 'MAIN') Then Result := @MainMenu;
End;


//====================================================================================================================//
// Main Loop ..

Procedure ODBG2_Pluginmainloop(Const DebugEvent: DEBUG_EVENT); Cdecl;
Begin
  Process;
End;


//====================================================================================================================//
// Free any memory ..

Procedure ODBG2_Plugindestroy; Cdecl;
Begin
  // Free ..
  Writetoini(Nil, Plugin_Name, 'Plugin is active', '%i', Int32(PluginActive));
End;


//====================================================================================================================//

Exports
  ODBG2_Pluginmainloop,
  ODBG2_Plugindestroy,
  ODBG2_Pluginmenu,
  ODBG2_Pluginquery,
  ODBG2_Plugininit;


//====================================================================================================================//

End.
