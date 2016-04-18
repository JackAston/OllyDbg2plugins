////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//                  SAMPLE BOOKMARK PLUGIN FOR OLLYDBG v2.01                  //
//                                                                            //
// This plugin allows to set up to 10 code bookmarks using keyboard shortcuts //
// or popup menus in Disassembler and then quickly return to one of the       //
// bookmarks using shortcuts, popup menu or Bookmark window. Bookmarks are    //
// preserved between sessions in the .udd files.                              //
//                                                                            //
// This code is distributed "as is", without warranty of any kind, expressed  //
// or implied, including, but not limited to warranty of fitness for any      //
// particular purpose. In no event will Oleh Yuschuk be liable to you for any //
// special, incidental, indirect, consequential or any other damages caused   //
// by the use, misuse, or the inability to use of this code, including any    //
// lost profits or lost savings, even if Oleh Yuschuk has been advised of the //
// possibility of such damages. Or, translated into English: use at your own  //
// risk!                                                                      //
//                                                                            //
// This code is free. You can modify it, include parts of it into your own    //
// programs and redistribute modified code provided that you remove all       //
// copyright messages or substitute them with your own copyright.             //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

// VERY IMPORTANT NOTICE: PLUGINS ARE UNICODE LIBRARIES! COMPILE THEM WITH BYTE
// ALIGNMENT OF STRUCTURES AND DEFAULT UNSIGNED CHAR!


#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <winnt.h>

#include "plugin.h"

#define PLUGINNAME     L"Bookmarks"    // Unique plugin name
#define VERSION        L"2.00.00"      // Plugin version

HINSTANCE        hdllinst;             // Instance of plugin DLL

// Most of OllyDbg windows are the so called tables. A table consists of table
// descriptor (t_table) with embedded sorted data (t_table.sorted, unused in
// custom tables). If data is present, all data elements have the same size and
// begin with a 3-dword t_sorthdr: address, size, type. Data is kept sorted by
// address (in our case this is the index of the bookmark), overlapping is not
// allowed. Our bookmark table consists of elements of type t_bookmark.
typedef struct t_bookmark {
  // Obligatory header, its layout _must_ coincide with t_sorthdr!
  ulong          index;                // Bookmark index (0..9)
  ulong          size;                 // Size of index, always 1 in our case
  ulong          type;                 // Type of entry, TY_xxx
  // Custom data.
  ulong          addr;                 // Address of bookmark
} t_bookmark;

static t_table   bookmark;             // Bookmark table
static int       showindisasm;         // Option: show bookmarks in Disasm pane


////////////////////////////////////////////////////////////////////////////////
//////////////////////////////// BOOKMARK TABLE ////////////////////////////////

// Function that compares two bookmarks according to the specified criterium.
// Returns -1 if first item is "lower" (should precede second on the screen),
// +1 if first item is "higher" and 0 if they are equal. Criterium n is the
// index of the column in the table.
int Bookmarksortfunc(const t_sorthdr *sh1,const t_sorthdr *sh2,const int n) {
  int i=0;
  t_bookmark *bm1,*bm2;
  bm1=(t_bookmark *)sh1;
  bm2=(t_bookmark *)sh2;
  if (n==1) {                          // Sort by address
    if (bm1->addr<bm2->addr) i=-1;
    else if (bm1->addr>bm2->addr) i=1; };
  if (i==0) {                          // Additionally sort by index
    if (bm1->index<bm2->index) i=-1;
    else if (bm1->index>bm2->index) i=1; };
  return i;
};

// Function that frees resources allocated in the descriptor of the bookmark.
// There are no such resources in the bookmark plugin, so this function could
// be declared as NULL instead.
void Bookmarkdestfunc(t_sorthdr *sh) {
};

// Drawing function of Bookmarks window.
int Bookmarkdraw(wchar_t *s,uchar *mask,int *select,
  t_table *pt,t_drawheader *ph,int column,void *cache) {
  int m,n;                             // Number of symbols in the string
  ulong length;
  uchar cmd[MAXCMDSIZE];
  t_bookmark *pmark;
  t_disasm *pasm;
  // For simple tables, t_drawheader is the pointer to the data element. It
  // can't be NULL, except in DF_CACHESIZE, DF_FILLCACHE and DF_FREECACHE.
  pmark=(t_bookmark *)ph;
  // Our cache is just a t_disasm. It is not NULL on the same conditions.
  pasm=(t_disasm *)cache;
  switch (column) {
    case DF_CACHESIZE:                 // Request for draw cache size
      // Columns 3 and 4 (disassembly and comment) both require calls to
      // Disasm(). To accelerate processing, I call disassembler once per line
      // and cache data between the calls. Here I inform the drawing routine
      // how large the cache must be.
      return sizeof(t_disasm);
    case DF_FILLCACHE:                 // Request to fill draw cache
      // We don't need to initialize cache when drawing begins. Note that cache
      // is initially zeroed.
      return 0;
    case DF_FREECACHE:                 // Request to free cached resources
      // We don't need to free cached resources when drawing ends.
      return 0;
    case DF_NEWROW:                    // Request to start new row in window
      // New row starts. Let us disassemble the command at the pointed address.
      // I assume that bookmarks can't be set on data. First of all, we need to
      // read the contents of memory. Length of 80x86 commands is limited to
      // MAXCMDSIZE bytes.
      length=Readmemory(cmd,pmark->addr,sizeof(cmd),MM_SILENT|MM_PARTIAL);
      if (length==0) {
        // Memory is not readable.
        StrcopyW(pasm->result,TEXTLEN,L"???");
        StrcopyW(pasm->comment,TEXTLEN,L""); }
      else
        Disasm(cmd,length,pmark->addr,Finddecode(pmark->addr,NULL),pasm,
        DA_TEXT|DA_OPCOMM|DA_MEMORY,NULL,NULL);
      return 0;
    case 0:                            // 0-based index
      n=StrcopyW(s,TEXTLEN,L"Alt+");
      memset(mask,DRAW_GRAY,n);
      m=swprintf(s+n,L"%i",pmark->index);
      memset(mask+n,DRAW_NORMAL,m);
      n+=m;
      *select|=DRAW_MASK;
      break;
    case 1:                            // Address of the bookmark
      n=Simpleaddress(s,pmark->addr,mask,select);
      break;
    case 2:                            // Disassembled command
      n=StrcopyW(s,TEXTLEN,pasm->result);
      break;
    case 3:                            // Comment
      // User-defined comment has highest priority.
      n=FindnameW(pmark->addr,NM_COMMENT,s,TEXTLEN);
      // Comment created by Disasm() is on the second place.
      if (n==0)
        n=StrcopyW(s,TEXTLEN,pasm->comment);
      // Analyser comment follows.
      if (n==0)
        n=Getanalysercomment(NULL,pmark->addr,s,TEXTLEN);
      // Procedure comments have the lowest priority.
      if (n==0)
        n=Commentaddress(pmark->addr,COMM_MARK|COMM_PROC,s,TEXTLEN);
      break;
    default: break;
  };
  return n;
};

// Custom table function of bookmarks window. Here it is used only to process
// doubleclicks (custom message WM_USER_DBLCLK). This function is also called
// on WM_DESTROY, WM_CLOSE (by returning -1, you can prevent window from
// closing), WM_SIZE (custom tables only), WM_CHAR (only if TABLE_WANTCHAR is
// set) and different custom messages WM_USER_xxx (depending on table type).
// See documentation for details.
long Bookmarkfunc(t_table *pt,HWND hw,UINT msg,WPARAM wp,LPARAM lp) {
  t_bookmark *pmark;
  switch (msg) {
    case WM_USER_DBLCLK:               // Doubleclick
      // Get selection.
      pmark=(t_bookmark *)Getsortedbyselection(
        &(pt->sorted),pt->sorted.selected);
      // Follow address in CPU Disassembler pane. Actual address is added to
      // the history, so that user can easily return back to it.
      if (pmark!=NULL) Setcpu(0,pmark->addr,0,0,0,
        CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
      return 1;
    default: break;
  };
  return 0;
};


////////////////////////////////////////////////////////////////////////////////
////////////////// PLUGIN MENUS EMBEDDED INTO OLLYDBG WINDOWS //////////////////

// Menu processing functions are called twice. First time (mode=MENU_VERIFY)
// OllyDbg asks to verify whether corresponding menu item applies or not. If
// necessary, menu function may change menu text (parameter name, up to TEXTLEN
// UNICODE characters). It must return one of the following codes:
//
//   MENU_ABSENT:     menu item does not apply and should not be present in
//                    the menu;
//   MENU_NORMAL:     menu item appears in the menu;
//   MENU_CHECKED:    menu item appears in the menu and has attached checkmark;
//   MENU_CHKPARENT:  menu item appears in the menu and has attached checkmark.
//                    Additionally, attaches checkmark to the parent item in
//                    menu on the previous level. This feature does not work in
//                    the main menu;
//   MENU_SHORTCUT:   menu item does not appear in the menu but is active and
//                    participates in the search for keyboard shortcut;
//   MENU_GRAYED:     item is present in the menu but disabled. This style is
//                    not compatible with OllyDbg's look-and-feel, use it only
//                    if absolutely necessary due to the menu logic.
//
// When menu item is selected (mouseclick or keyboard shortcut), menu function
// is called for the second time (mode=MENU_EXECUTE, name is undefined). At
// this moment, all threads of the debugged application are suspended. Function
// must make all necessary actions and return one of the following codes:
//
//   MENU_REDRAW:     this action has global impact, all OllyDbg windows must
//                    be updated. OllyDbg broadcasts WM_USER_CHGALL;
//   MENU_NOREDRAW:   no redrawing is necessary.
//
// If processing is lengthy and application should continue execution, use
// Resumeallthreads() at entry to the MENU_EXECUTE block and Suspendallthreads()
// on exit. Note that MENU_ABSENT and MENU_NOREDRAW are interchangeable.
//
// Parameter index allows to use single menu function with several similar menu
// items.
//
// Note that OllyDbg uses menu structuress to process keyboard shortcuts. It is
// done automatically, without the need to pay additional attention.

// Menu function of main menu, opens or brings to top list of bookmarks.
static int Mopenbookmarks(t_table *pt,wchar_t *name,ulong index,int mode) {
  if (mode==MENU_VERIFY)
    return MENU_NORMAL;                // Always available
  else if (mode==MENU_EXECUTE) {
    if (bookmark.hw==NULL)
      // Create table window. Third parameter (ncolumn) is the number of
      // visible columns in the newly created window (ignored if appearance is
      // restored from the initialization file). If it's lower than the total
      // number of columns, remaining columns are initially invisible. Fourth
      // parameter is the name of icon - as OllyDbg resource.
      Createtablewindow(&bookmark,0,bookmark.bar.nbar,NULL,L"ICO_P",PLUGINNAME);
    else
      Activatetablewindow(&bookmark);
    return MENU_NOREDRAW;
  };
  return MENU_ABSENT;
};

// Menu function of main menu, shows or hides bookmark display in the
// Disassembler pane.
static int Mshowbookmarks(t_table *pt,wchar_t *name,ulong index,int mode) {
  if (mode==MENU_VERIFY)
    return (showindisasm?MENU_CHECKED:MENU_NORMAL);
  else if (mode==MENU_EXECUTE) {
    showindisasm=!showindisasm;
    Writetoini(NULL,PLUGINNAME,L"Show bookmarks in Disassembler",L"%i",
      showindisasm);
    return MENU_REDRAW;
  };
};

// Menu function of main menu, displays About dialog.
static int Mabout(t_table *pt,wchar_t *name,ulong index,int mode) {
  int n;
  wchar_t s[TEXTLEN];
  if (mode==MENU_VERIFY)
    return MENU_NORMAL;                // Always available
  else if (mode==MENU_EXECUTE) {
    // Debuggee should continue execution while message box is displayed.
    Resumeallthreads();
    // In this case, swprintf() would be as good as a sequence of StrcopyW(),
    // but secure copy makes buffer overflow impossible.
    n=StrcopyW(s,TEXTLEN,L"Bookmark plugin v");
    n+=StrcopyW(s+n,TEXTLEN-n,VERSION);
    // COPYRIGHT POLICY: This bookmark plugin is an open-source freeware. It's
    // just a sample. The copyright below is also just a sample and applies to
    // the unmodified sample code only. Replace or remove copyright message if
    // you make ANY changes to this code!
    StrcopyW(s+n,TEXTLEN-n,L"\nCopyright (C) 2001-2011 Oleh Yuschuk");
    MessageBox(hwollymain,s,
      L"Bookmark plugin",MB_OK|MB_ICONINFORMATION);
    // Suspendallthreads() and Resumeallthreads() must be paired, even if they
    // are called in inverse order!
    Suspendallthreads();
    return MENU_NOREDRAW;
  };
  return MENU_ABSENT;
};

// Plugin menu that will appear in the main OllyDbg menu. Note that this menu
// must be static and must be kept for the whole duration of the debugging
// session.
static t_menu mainmenu[] = {
  { L"Open bookmarks",
       L"Open Bookmarks window",
       K_NONE, Mopenbookmarks, NULL, 0 },
  { L"Show bookmarks in Disassembler",
       L"Mark bookmarked command in the Disassembler pane of CPU window",
       K_NONE, Mshowbookmarks, NULL, 0 },
  { L"|About",
       L"About Bookmarks plugin",
       K_NONE, Mabout, NULL, 0 },
  { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

// Menu function of Disassembler pane that creates new bookmark.
static int Msetmark(t_table *pt,wchar_t *name,ulong index,int mode) {
  t_bookmark bmk,*pmark;
  t_dump *pd;
  // Get dump descriptor. This operation is common for both calls.
  if (pt==NULL || pt->customdata==NULL)
    return MENU_ABSENT;
  pd=(t_dump *)pt->customdata;
  if (mode==MENU_VERIFY) {
    // Check whether selection is defined.
    if (pd->sel0>=pd->sel1)
      return MENU_ABSENT;
    // Check whether bookmark with given index already exists.
    pmark=(t_bookmark *)Findsorteddata(&(bookmark.sorted),index,0);
    if (pmark!=NULL)
      return MENU_ABSENT;
    swprintf(name,L"Set bookmark %i",index);
    return MENU_NORMAL; }
  else if (mode==MENU_EXECUTE) {
    bmk.index=index;
    bmk.size=1;
    bmk.type=0;
    bmk.addr=pd->sel0;
    Addsorteddata(&(bookmark.sorted),&bmk);
    Pluginmodulechanged(pd->sel0);
    return MENU_REDRAW; };
  return MENU_ABSENT;
};

// Menu function of Disassembler pane that follows existing bookmark.
static int Mfollowmark(t_table *pt,wchar_t *name,ulong index,int mode) {
  t_bookmark *pmark;
  t_dump *pd;
  // Get dump descriptor.
  if (pt==NULL || pt->customdata==NULL)
    return MENU_ABSENT;
  pd=(t_dump *)pt->customdata;
  // Check whether bookmark is defined.
  pmark=(t_bookmark *)Findsorteddata(&(bookmark.sorted),index,0);
  if (pmark==NULL)
    return MENU_ABSENT;
  if (mode==MENU_VERIFY) {
    // If we are already at bookmark, precede its name with checkmark.
    if (pd->sel0<pd->sel1 && pmark->addr==pd->sel0) {
      swprintf(name,L"Bookmark %i",index);
      return MENU_CHECKED; };
    swprintf(name,L"Follow bookmark %i",index);
    return MENU_NORMAL; }
  else if (mode==MENU_EXECUTE) {
    Setcpu(0,pmark->addr,0,0,0,
      CPU_ASMHIST|CPU_ASMCENTER|CPU_ASMFOCUS);
    return MENU_REDRAW; };
  return MENU_ABSENT;
};

// Menu function of Disassembler pane that deletes existing bookmark.
static int Mdeletemark(t_table *pt,wchar_t *name,ulong index,int mode) {
  t_bookmark *pmark;
  t_dump *pd;
  // Get dump descriptor.
  if (pt==NULL || pt->customdata==NULL)
    return MENU_ABSENT;
  pd=(t_dump *)pt->customdata;
  if (mode==MENU_VERIFY) {
    // Check whether bookmark is defined.
    pmark=(t_bookmark *)Findsorteddata(&(bookmark.sorted),index,0);
    if (pmark==NULL)
      return MENU_ABSENT;
    swprintf(name,L"Delete bookmark %i",index);
    // If we are already at bookmark, precede its name with checkmark.
    if (pd->sel0<pd->sel1 && pmark->addr==pd->sel0) {
      return MENU_CHECKED; };
    return MENU_NORMAL; }
  else if (mode==MENU_EXECUTE) {
    Deletesorteddata(&(bookmark.sorted),index,0);
    return MENU_REDRAW; };
  return MENU_ABSENT;
};

// Plugin menu that will appear in the Disassembler pane of CPU window.
static t_menu disasmmenu[] = {
  // Menu items that set new bookmarks. Note that their names will be created
  // dynamically by Msetmark().
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'0', Msetmark, NULL, 0 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'1', Msetmark, NULL, 1 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'2', Msetmark, NULL, 2 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'3', Msetmark, NULL, 3 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'4', Msetmark, NULL, 4 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'5', Msetmark, NULL, 5 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'6', Msetmark, NULL, 6 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'7', Msetmark, NULL, 7 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'8', Msetmark, NULL, 8 },
  { L"",
       L"Set new bookmark at selection",
       KK_DIRECT|KK_CTRL|'9', Msetmark, NULL, 9 },
  // Menu items that follow existing bookmarks. Their names will be created
  // dynamically by Mfollowmark(). Note separator line introduced by the first
  // item.
  { L"|",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'0', Mfollowmark, NULL, 0 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'1', Mfollowmark, NULL, 1 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'2', Mfollowmark, NULL, 2 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'3', Mfollowmark, NULL, 3 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'4', Mfollowmark, NULL, 4 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'5', Mfollowmark, NULL, 5 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'6', Mfollowmark, NULL, 6 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'7', Mfollowmark, NULL, 7 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'8', Mfollowmark, NULL, 8 },
  { L"",
       L"Follow bookmark",
       KK_DIRECT|KK_ALT|'9', Mfollowmark, NULL, 9 },
  // Menu items that delete existing bookmarks.
  { L"|",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'0', Mdeletemark, NULL, 0 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'1', Mdeletemark, NULL, 1 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'2', Mdeletemark, NULL, 2 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'3', Mdeletemark, NULL, 3 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'4', Mdeletemark, NULL, 4 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'5', Mdeletemark, NULL, 5 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'6', Mdeletemark, NULL, 6 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'7', Mdeletemark, NULL, 7 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'8', Mdeletemark, NULL, 8 },
  { L"",
       L"Delete bookmark",
       KK_DIRECT|KK_CTRL|KK_ALT|'9', Mdeletemark, NULL, 9 },
  // End of menu.
  { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

// Adds items either to main OllyDbg menu (type=PWM_MAIN) or to popup menu in
// one of the standard OllyDbg windows, like PWM_DISASM or PWM_MEMORY. When
// type matches, plugin should return address of menu. When there is no menu of
// given type, it must return NULL. If menu includes single item, it will
// appear directly in menu, otherwise OllyDbg will create a submenu with the
// name of plugin. Therefore, if there is only one item, make its name as
// descriptive as possible.
extc t_menu * _export cdecl ODBG2_Pluginmenu(wchar_t *type) {
  if (wcscmp(type,PWM_MAIN)==0)
    // Main menu.
    return mainmenu;
  else if (wcscmp(type,PWM_DISASM)==0)
    // Disassembler pane of CPU window.
    return disasmmenu;
  return NULL;                         // No menu
};


////////////////////////////////////////////////////////////////////////////////
///////////////////////////// BOOKMARK WINDOW MENU /////////////////////////////

// If item name has form >STANDARD, >FULLCOPY or >APPEARANCE, this item is a
// forwarder to the standard OllyDbg table menu or its part. Use forwarders
// only in windows created by plugin, never in menus that plug into the native
// OllyDbg windows.
static t_menu bookmarkmenu[] = {       // Menu of the bookmark window
  { L"|>STANDARD",
       L"",                            // Forwarder to standard menus
       K_NONE, NULL, NULL, 0
  }
};


////////////////////////////////////////////////////////////////////////////////
/////////////////////////////// DUMP WINDOW HOOK ///////////////////////////////

// Dump windows display contents of memory or file as bytes, characters,
// integers, floats or disassembled commands. Plugins have the option to modify
// the contents of the dump windows. If ODBG2_Plugindump() is present and some
// dump window is being redrawn, this function is called first with column=
// DF_FILLCACHE, addr set to the address of the first visible element in the
// dump window and n to the estimated total size of the data displayed in the
// window (n may be significantly higher than real data size for disassembly).
// If plugin returns 0, there are no elements that will be modified by plugin
// and it will receive no other calls. If necessary, plugin may cache some data
// necessary later. OllyDbg guarantees that there are no calls to
// ODBG2_Plugindump() from other dump windows till the final call with
// DF_FREECACHE.
// When OllyDbg draws table, there is one call for each table cell (line/column
// pair). Parameters s (UNICODE), mask (DRAW_xxx) and select (extended DRAW_xxx
// set) contain description of the generated contents of length n. Plugin may
// modify it and return corrected length, or just return the original length.
// When table is completed, ODBG2_Plugindump() receives final call with
// column=DF_FREECACHE. This is the time to free resources allocated on
// DF_FILLCACHE. Returned value is ignored.
// Use this feature only if absolutely necessary, because it may strongly
// impair the responsiveness of the OllyDbg. Always make it switchable with
// default set to OFF!
extc int _export cdecl ODBG2_Plugindump(t_dump *pd,
  wchar_t *s,uchar *mask,int n,int *select,ulong addr,int column) {
  int i,j;
  wchar_t w[TEXTLEN];
  t_bookmark *pmark;
  if (column==DF_FILLCACHE) {
    // Check whether this feature is requested.
    if (showindisasm==0)
      return 0;                        // Turned off
    // Check whether it's Disassembler pane of the CPU window.
    if (pd==NULL || (pd->menutype & DMT_CPUMASK)!=DMT_CPUDASM)
      return 0;                        // Not a Disassembler
    // Just for the sake, assure that bookmarks apply: not a file dump, not a
    // backup display
    if (pd->filecopy!=NULL ||
      (pd->dumptype & DU_TYPEMASK)!=DU_DISASM ||
      (pd->dumptype & DU_BACKUP)!=0)
      return 0;                        // Invalid dump type
    // Check that there are bookmarks in the supplied memory range.
    for (i=0; i<bookmark.sorted.n; i++) {
      pmark=(t_bookmark *)Getsortedbyindex(&(bookmark.sorted),i);
      if (pmark==NULL)
        continue;                      // Must not happen!
      if (pmark->addr>=addr && pmark->addr<addr+n)
        return 1;                      // Bookmark to display
      ;
    };
    return 0; }                        // No bookmarks to display
  else if (column==2) {
    // Check whether there is a bookmark. Note that there may be several marks
    // on the same address!
    w[0]=L'['; j=1;
    for (i=0; i<bookmark.sorted.n; i++) {
      pmark=(t_bookmark *)Getsortedbyindex(&(bookmark.sorted),i);
      if (pmark==NULL)
        continue;                      // Must not happen!
      if (pmark->addr!=addr)
        continue;
      if (j>1)
        w[j++]=L',';                   // Comma between bookmarks
      j+=swprintf(w+j,L"%i",pmark->index);
    };
    if (j==1)
      return n;                        // No bookmarks on address
    j+=StrcopyW(w+j,TEXTLEN-j,L"] ");
    // Disassebly column is always drawn by mask (DRAW_MASK bit set). But this
    // is a sample plugin and I show here what to do if this is not the case.
    if ((*select & DRAW_MASK)==0) {
      memset(mask,DRAW_NORMAL,n);
      *select|=DRAW_MASK; };
    // Skip graphical symbols (loop brackets).
    for (i=0; i<n; i++) {
      if ((mask[i] & DRAW_GRAPH)==0) break; };
    // Insert text.
    n+=j; if (n>TEXTLEN) n=TEXTLEN;
    if (n>i+j) {
      memmove(s+i+j,s+i,(n-i-j)*sizeof(wchar_t));
      memmove(mask+i+j,mask+i,n-i-j); };
    memcpy(s+i,w,j*sizeof(wchar_t));
    memset(mask+i,DRAW_EIP,j-1);
    mask[i+j-1]=DRAW_NORMAL; }
  else if (column==DF_FREECACHE) {
    // We have allocated no resources, so we have nothing to do here.
  };
  return n;
};


////////////////////////////////////////////////////////////////////////////////
//////////////////////////// PLUGIN INITIALIZATION /////////////////////////////

// Entry point into a plugin DLL. Many system calls require DLL instance
// which is passed to DllEntryPoint() as one of parameters. Remember it.
// Preferrable way for initializations is to place them into ODBG_Plugininit()
// and cleanup in ODBG_Plugindestroy().
BOOL WINAPI DllEntryPoint(HINSTANCE hi,DWORD reason,LPVOID reserved) {
  if (reason==DLL_PROCESS_ATTACH)
    hdllinst=hi;                       // Mark plugin instance
  return 1;                            // Report success
};

// ODBG_Pluginquery() is a "must" for valid OllyDbg plugin. First it must check
// whether given OllyDbg version is correctly supported, and return 0 if not.
// Then it should make one-time initializations and allocate resources. On
// error, it must clean up and return 0. On success, if should fill plugin name
// and plugin version (as UNICODE strings) and return version of expected
// plugin interface. If OllyDbg decides that this plugin is not compatible, it
// will call ODBG2_Plugindestroy() and unload plugin. Plugin name identifies it
// in the Plugins menu. This name is max. 31 alphanumerical UNICODE characters
// or spaces + terminating L'\0' long. To keep life easy for users, this name
// should be descriptive and correlate with the name of DLL. This function
// replaces ODBG_Plugindata() and ODBG_Plugininit() from the version 1.xx.
extc int _export cdecl ODBG2_Pluginquery(int ollydbgversion,
  wchar_t pluginname[SHORTNAME],wchar_t pluginversion[SHORTNAME]) {
  int restore;
  // Check whether OllyDbg has compatible version. This plugin uses only the
  // most basic functions, so this check is done pro forma, just to remind of
  // this option.
  if (ollydbgversion<201)
    return 0;
  // Initialize bookmark storage. Data contains no resources, so destructor is
  // not necessary. (Destructor is called each time data item is removed from
  // the sorted data). I add it pro forma, just to remind you of this option.
  Createsorteddata(&(bookmark.sorted), // Descriptor of sorted data
    sizeof(t_bookmark),                // Size of single data item
    10,                                // Initial number of allocated items
    (SORTFUNC *)Bookmarksortfunc,      // Sorting function
    (DESTFUNC *)Bookmarkdestfunc,      // Data destructor
    0);                                // Simple data, no special options
  // Initialize bookmark table. OllyDbg uses table name to save the appearance
  // to the main initialization file. Keep this name unique, or else.
  StrcopyW(bookmark.name,SHORTNAME,PLUGINNAME);
  bookmark.mode=TABLE_SAVEALL;         // Save complete appearance
  bookmark.bar.visible=1;              // By default, bar is visible
  bookmark.bar.name[0]=L"Bookmark";
  bookmark.bar.expl[0]=L"Bookmark index";
  bookmark.bar.mode[0]=BAR_SORT;
  bookmark.bar.defdx[0]=9;
  bookmark.bar.name[1]=L"Address";
  bookmark.bar.expl[1]=L"Bookmark address";
  bookmark.bar.mode[1]=BAR_SORT;
  bookmark.bar.defdx[1]=9;
  bookmark.bar.name[2]=L"Disassembly";
  bookmark.bar.expl[2]=L"Command at the bookmark address";
  bookmark.bar.mode[2]=BAR_FLAT;
  bookmark.bar.defdx[2]=24;
  bookmark.bar.name[3]=L"Comments";
  bookmark.bar.expl[3]=L"Comments";
  bookmark.bar.mode[3]=BAR_FLAT;
  bookmark.bar.defdx[3]=256;
  bookmark.bar.nbar=4;
  bookmark.tabfunc=Bookmarkfunc;
  bookmark.custommode=0;
  bookmark.customdata=NULL;
  bookmark.updatefunc=NULL;
  bookmark.drawfunc=(DRAWFUNC *)Bookmarkdraw;
  bookmark.tableselfunc=NULL;
  bookmark.menu=bookmarkmenu;
  // Get initialization data.
  showindisasm=0;                      // Default value
  Getfromini(NULL,PLUGINNAME,L"Show bookmarks in Disassembler",L"%i",
    &showindisasm);
  // OllyDbg saves positions of plugin windows with attribute TABLE_SAVEPOS to
  // the .ini file but does not automatically restore them. Let us add this
  // functionality here. To conform to OllyDbg norms, window is restored only
  // if corresponding option is enabled.
  if (restorewinpos!=0) {
    restore=0;                         // Default
    Getfromini(NULL,PLUGINNAME,L"Restore window",L"%i",&restore);
    if (restore)
      Createtablewindow(&bookmark,0,bookmark.bar.nbar,NULL,L"ICO_P",PLUGINNAME);
    ;
  };
  // Report name and version to OllyDbg.
  wcscpy(pluginname,PLUGINNAME);       // Name of plugin
  wcscpy(pluginversion,VERSION);       // Version of plugin
  return PLUGIN_VERSION;
};

// If you define ODBG2_Pluginmainloop, this function will be called each time
// from the main Windows loop in OllyDbg. If there is some (real) debug event
// from the debugged application, debugevent points to it, otherwise it's NULL.
// If fast command emulation is active, it does not receive all (emulated)
// exceptions, use ODBG2_Pluginexception() instead. Do not declare this
// function unnecessarily, as this may negatively influence the overall speed!
extc void _export cdecl ODBG2_Pluginmainloop(DEBUG_EVENT *debugevent) {


};


extc void _export cdecl ODBG2_Pluginexception(t_reg *preg) {


};




// Optional entry, called each time OllyDbg analyses some module and analysis
// is finished. Plugin can make additional analysis steps. Debugged application
// is paused for the time of processing. Bookmark plugin, of course, does not
// analyse code. If you don't need this feature, remove ODBG2_Pluginanalyse()
// from the plugin code.
extc void _export cdecl ODBG2_Pluginanalyse(t_module *pmod) {
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////// .UDD FILES //////////////////////////////////

// For each plugin, OllyDbg automatically creates file-unique block of .udd
// record types. They are indexed by plugin names. If there are two plugins
// with the same name, restored records may be sent to the wrong plugin.
// Therefore it is very important that plugin names are unique.

#define BOOKMARKTAG    0x0001          // Tag of bookmark record

typedef struct t_uddmark {             // Bookmark in .udd file
  int            index;                // Bookmark index
  ulong          offset;               // Offset from module base
} t_uddmark;

// Time to save data to .udd file! This is done by calling Pluginsaverecord()
// for each data item that must be saved. Global, process-oriented data must
// be saved in main .udd file (named by .exe); module-relevant data must be
// saved in module-related files. Don't forget to save all addresses relative
// to module's base, so that data will be restored correctly even when module
// is relocated.
extc void _export cdecl ODBG2_Pluginsaveudd(t_uddsave *psave,t_module *pmod,
  int ismainmodule) {
  int i;
  t_bookmark *pmark;
  t_uddmark um;
  for (i=0; i<bookmark.sorted.n; i++) {
    pmark=(t_bookmark *)Getsortedbyindex(&(bookmark.sorted),i);
    if (pmark==NULL)
      continue;                        // Must not happen!
    if (pmark->addr>=pmod->base && pmark->addr<pmod->base+pmod->size) {
      // Bookmarks are saved with corresponding modules. If bookmark was set
      // outside of any module, it will be lost.
      um.index=pmark->index;
      um.offset=pmark->addr-pmod->base;
      Pluginsaverecord(psave,BOOKMARKTAG,sizeof(um),&um);
    };
  };
};

// OllyDbg restores data from .udd file and has encountered record belonging
// to our plugin. Note that module descriptor pointed to by pmod can be
// incomplete, i.e. does not necessarily contain all informations, especially
// that from .udd file.
extc void _export cdecl ODBG2_Pluginuddrecord(t_module *pmod,int ismainmodule,
  ulong tag,ulong size,void *data) {
  t_bookmark bmk;
  t_uddmark *pum;
  if (pmod==NULL || data==NULL)
    return;                            // Must not happen!
  if (size!=sizeof(t_uddmark))
    return;                            // Invalid record size
  if (tag!=BOOKMARKTAG)
    return;                            // Invalid tag
  pum=(t_uddmark *)data;
  if (pum->index<0 || pum->index>9)
    return;                            // Corrupted data
  bmk.index=pum->index;
  bmk.size=1;
  bmk.type=0;
  bmk.addr=pmod->base+pum->offset;
  Addsorteddata(&(bookmark.sorted),&bmk);
};


////////////////////////////////////////////////////////////////////////////////



// Function is called when user opens new or restarts current application.
// Plugin should reset internal variables and data structures to initial state.
extc void _export cdecl ODBG2_Pluginreset(void) {
  Deletesorteddatarange(&(bookmark.sorted),0,0xFFFFFFFF);
};

// OllyDbg calls this optional function when user wants to terminate OllyDbg.
// All MDI windows created by plugins still exist. Function must return 0 if
// it is safe to terminate. Any non-zero return will stop closing sequence. Do
// not misuse this possibility! Always inform user about the reasons why
// termination is not good and ask for his decision! Attention, don't make any
// unrecoverable actions for the case that some other plugin will decide that
// OllyDbg should continue running.
extc int _export cdecl ODBG2_Pluginclose(void) {
  // For automatical restoring of open windows, mark in .ini file whether
  // Bookmarks window is still open.
  Writetoini(NULL,PLUGINNAME,L"Restore window",L"%i",bookmark.hw!=NULL);
  return 0;
};

// OllyDbg calls this optional function once on exit. At this moment, all MDI
// windows created by plugin are already destroyed (and received WM_DESTROY
// messages). Function must free all internally allocated resources, like
// window classes, files, memory etc.
extc void _export cdecl ODBG2_Plugindestroy(void) {
  Destroysorteddata(&(bookmark.sorted));
};

