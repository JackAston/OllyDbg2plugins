
/**
 * OllyDetach - OllyDbg 2.x plugin
 *
 *  v2 - coded by atom0s
 *  v1 - coded by Pedram Amini
 *
 * Full credit for this plugin goes to Pedram Amini. I, atom0s, simply
 * updated it for OllyDbg 2.x
 *
 */

#include <Windows.h>
#include <TlHelp32.h>

#pragma comment( lib, "ollydbg.lib" )
#include "plugin.h"


/**
 * Global Variables
 */
HINSTANCE g_vPluginInstance = NULL;

/**
 * Plugin Information
 */
wchar_t g_vPluginName[SHORTNAME]      = L"OllyDetach";
wchar_t g_vPluginVersion[SHORTNAME]   = L"2.0.0";

/**
 * Plugin Menu
 */
static int menu_handler( t_table* pTable, wchar_t* pName, ulong index, int nMode );
static t_menu g_vMainMenu[] =
{
    { L"Detach From Debugee",
        L"Detachs OllyDbg from the current debugee.",
        K_NONE, menu_handler, NULL, 0 },
    { L"About", 
        L"About OllyDetach",  
        K_NONE, menu_handler, NULL, 1 },

    { NULL, NULL, K_NONE, NULL, NULL, NULL }
};


/**
 * @DllMain
 * 
 *  Unused Entrypoint
 */
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
    UNREFERENCED_PARAMETER( lpvReserved );

    switch( fdwReason )
    {
    case DLL_PROCESS_ATTACH:
        g_vPluginInstance = hinstDLL;
        break;
    }
    return TRUE;
}

/**
 * @ODBG2_Pluginquery
 *
 *  Handles initializing the plugin.
 */
extc int _export cdecl ODBG2_Pluginquery( int ollydbgversion, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME] )
{
    // Validate plugin version..
    if( ollydbgversion != PLUGIN_VERSION )
        return 0;

    // Copy plugin information..
    wcscpy_s( pluginname, SHORTNAME, g_vPluginName );
    wcscpy_s( pluginversion, SHORTNAME, g_vPluginVersion );
    
    return PLUGIN_VERSION;
}

/**
 * @ODBG2_Pluginmenu
 * 
 *  Adds items to OllyDbgs menu system.
 */
extc _export t_menu* cdecl ODBG2_Pluginmenu( wchar_t* type )
{
    if( ! _wcsnicmp( type, PWM_MAIN, SHORTNAME ) )
        return g_vMainMenu;
    return NULL;
}

/**
 * @detach_from_debugee
 * 
 *  Attempts to detach from the debugee process.
 */
bool detach_from_debugee( void )
{
    // Obtain OllyDbg handle..
    HMODULE hModule = GetModuleHandle( 0 );
    if( hModule == NULL ) 
    { 
        Addtolist( 0, 0, L"OllyDetach - Failed to obtain OllyDbg handle.." );
        return false;
    }

    // Obtain Detachprocess function pointer..
    FARPROC fpFuncAddress = GetProcAddress( hModule, "Detachprocess" );
    if( fpFuncAddress == NULL )
    {
        Addtolist( 0, 0, L"OllyDetach - Failed to find Detachprocess function.." );
        return false;
    }

    // Attempt to call function..
    ( ( int ( WINAPI *)( void ) )fpFuncAddress )( );
    return true;
}

/**
 * @menu_handler
 *
 *  Handles this plugins menu callbacks.
 */
static int menu_handler( t_table* pTable, wchar_t* pName, ulong index, int nMode )
{
    UNREFERENCED_PARAMETER( pTable );
    UNREFERENCED_PARAMETER( pName );

    switch( nMode )
    {
    case MENU_VERIFY:
        return MENU_NORMAL;

    case MENU_EXECUTE:
        {
            switch( index )
            {
            case 0: // Detach from debugee
                detach_from_debugee();
                break;
            case 1: // About
                MessageBox( hwollymain,
                    L"OllyDetach - v2.0.0\r\nv2 - coded by atom0s\r\nv1 - coded by Pedram Amini\r\n\r\nFull credit to Pedram Amini for this plugin!",
                    L"OllyDetach",
                    MB_ICONINFORMATION | MB_OK
                    );
                break;
            }
            return MENU_NOREDRAW;
        }
    }

    return MENU_ABSENT;
}