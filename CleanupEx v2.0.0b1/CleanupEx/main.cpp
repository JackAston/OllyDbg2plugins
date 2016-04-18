/**
 * CleanupEx - OllyDbg 2.x plugin
 *
 * v2 - Coded by atom0s
 * v1 - Coded by Gigapede
 *
 *
 * v2 - Updated for OllyDbg 2.x
 * 
 * Some features are missing; mainly debugee support and confirm delete removal.
 *
 */

#pragma comment( lib, "ollydbg.lib" )

#include <Windows.h>
#include <stdio.h>
#include "plugin.h"
#include "resource.h"

#include "menu_handler.h"

/**
 * Plugin Information Variables
 */
HINSTANCE   g_vPluginInstance   = NULL;
wchar_t     g_vPluginName[ 255 ];
wchar_t     g_vPluginVersion[ 255 ];
wchar_t     g_vConfirmDelete[ 255 ];
wchar_t     g_vAboutMessage[ 255 ];

/**
 * Global Storage Variables
 */
std::wstring g_vDataDirectory   = std::wstring(L"");

/**
 * Main Menu Definition
 */
static t_menu g_vMainMenu[]     =
{
    { L"Delete All (*.udd, *.bak)", 
        L"Deletes all session files.", 
        K_NONE, menu_handler, NULL, 0 },
    { L"Delete Backup Only (*.bak)", 
        L"Deletes only backup files.", 
        K_NONE, menu_handler, NULL, 1 },
    { L"Delete Session Only (*.udd)", 
        L"Deletes only session files.", 
        K_NONE, menu_handler, NULL, 2 },
    { L"|", 
        L"", 
        K_NONE, NULL, NULL, 0 },
    { L"About", 
        L"About CleanupEx",  
        K_NONE, menu_handler, NULL, 3 },
    
    { NULL, NULL, K_NONE, NULL, NULL, 0 }
};

/**
 * @ODBG2_Pluginquery - required!
 * 
 *      Handles initializing the plugin.
 */
extc int __cdecl ODBG2_Pluginquery( int ollydbgversion, ulong* features, wchar_t pluginname[SHORTNAME], wchar_t pluginversion[SHORTNAME] )
{
	UNREFERENCED_PARAMETER( features );

    // Validate plugin version..
    //if( ollydbgversion != PLUGIN_VERSION )
    //    return 0;

	if( ollydbgversion < 201 )
		return 0;

    // Read plugin name and version from resource file..
    LoadString( g_vPluginInstance, IDS_PLUGIN_NAME, g_vPluginName, 255 );
    LoadString( g_vPluginInstance, IDS_PLUGIN_VERSION, g_vPluginVersion, 255 );
    LoadString( g_vPluginInstance, IDS_CONFIRM_DELETE, g_vConfirmDelete, 255 );
    LoadString( g_vPluginInstance, IDS_PLUGIN_ABOUT, g_vAboutMessage, 255 );

    // Set plugin name and version..
    wcscpy_s( pluginname, SHORTNAME, g_vPluginName );
    wcscpy_s( pluginversion, SHORTNAME, g_vPluginVersion );

    // Obtain UDD Path (Data Directory)
    wchar_t wszDataDirectory[ MAX_PATH ]    = { 0 };
    wchar_t wszFullDataPath[ MAX_PATH ]     = { 0 };
    Getfromini( NULL, L"HISTORY", L"Data directory", L"%255s", &wszDataDirectory );
    GetFullPathName( wszDataDirectory, MAX_PATH, wszFullDataPath, NULL );
    g_vDataDirectory = wszFullDataPath;

    return PLUGIN_VERSION;
}

/**
 * @ODBG2_Pluginmenu
 *
 *      Adds items to OllyDbgs menu system.
 */
extc t_menu* __cdecl ODBG2_Pluginmenu( wchar_t* type )
{
    if( ! _wcsnicmp( type, PWM_MAIN, SHORTNAME ) )
        return g_vMainMenu;
    return NULL;
}

/**
 * @DllMain
 * 
 *      Dll entrypoint - mainly unused.
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