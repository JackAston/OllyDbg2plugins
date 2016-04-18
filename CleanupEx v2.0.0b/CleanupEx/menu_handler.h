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

#pragma once

#ifndef __MENU_HANDLER_INCLUDED__
#define __MENU_HANDLER_INCLUDED__

#include <Windows.h>
#include <string>
#include <list>
#include "plugin.h"

#include "resource.h"

/**
 * Externs
 */
extern std::wstring g_vDataDirectory;
extern wchar_t g_vPluginName[255];
extern wchar_t g_vPluginVersion[255];
extern wchar_t g_vConfirmDelete[255];
extern wchar_t g_vAboutMessage[255];

/**
 * Delete Type Enumeration
 */
enum delete_t 
{
    DELETE_ALL = 0,
    DELETE_BAK = 1,
    DELETE_UDD = 2
};

/**
 * @DeleteFiles
 *
 *      Attempts to delete the files with the given extension type.
 *      See delete_t for types.
 */
bool DeleteFiles( delete_t delType )
{
    // Validate data directory path is set..
    if( g_vDataDirectory.length() == 0 || GetFileAttributes( g_vDataDirectory.c_str() ) == 0xFFFFFFFF )
    {
        Addtolist( 0, 2, L"[CleanupEx] Error: Cannot delete selected files; data directory is invalid." );
        return false;
    }

    // Build extension list..
    std::list< wchar_t* > lstExtensions;
    switch( delType )
    {
    case DELETE_ALL:
        lstExtensions.push_back( L"*.udd" );
        lstExtensions.push_back( L"*.bak" );
        break;
    case DELETE_BAK:
        lstExtensions.push_back( L"*.bak" );
        break;
    case DELETE_UDD:
        lstExtensions.push_back( L"*.udd" );
        break;
    }

    // Loop the delete type list and attempt to delete found files..
    for( std::list< wchar_t* >::iterator iter = lstExtensions.begin(), iterend = lstExtensions.end();
         iter != iterend; iter++ )
    {
        // Build proper path with mask..
        wchar_t wszDataDirectory[ MAX_PATH ] = { 0 };
        _snwprintf_s( wszDataDirectory, MAX_PATH, L"%s\\%s", g_vDataDirectory.c_str(), ( *iter ) );

        // Loop the directory and find our files..
        WIN32_FIND_DATA wfd = { 0 };
        HANDLE hSnapshot = FindFirstFile( wszDataDirectory, &wfd );
        if( hSnapshot == INVALID_HANDLE_VALUE )
            continue;

        do
        {
            // Display current file..
            Addtolist( 0, 0, L"Processing file: %s", wfd.cFileName );

            // Build confirm message..
            wchar_t wszDeleteConfirm[ 255 ] = { 0 };
            _snwprintf_s( wszDeleteConfirm, 255, g_vConfirmDelete, wfd.cFileName );

            // Display confirm message..
            if( MessageBox( hwollymain, wszDeleteConfirm, L"Delete Confirmation", MB_YESNO | MB_ICONQUESTION ) == IDYES )
            {
                // Attempt to delete the file..
                wchar_t wszFullFilePath[ MAX_PATH ] = { 0 };
                _snwprintf_s( wszFullFilePath, MAX_PATH, L"%s\\%s", g_vDataDirectory.c_str(), wfd.cFileName );
                if( ! DeleteFile( wszFullFilePath ) )
                    Addtolist( 0, 0, L"Failed to delete the file: %s", wfd.cFileName );
            }

        } while( FindNextFile(hSnapshot, &wfd ) );

        // Cleanup..
        FindClose( hSnapshot );
        hSnapshot = INVALID_HANDLE_VALUE;
    }

    return false;
}

/**
 * @menu_handler
 * 
 *      Menu callback for our plugin to process our menu commands.
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
            case 0: // Menu -> Delete All
            case 1: // Menu -> Delete BAK
            case 2: // Menu -> Delete UDD
                DeleteFiles( static_cast< delete_t >( index ) );
                break;
            case 3: // Menu -> About
                {
                    wchar_t wszAboutMessage[ 255 ] = { 0 };
                    _snwprintf_s( wszAboutMessage, 255, g_vAboutMessage, g_vPluginName, g_vPluginVersion );
                    MessageBox( hwollymain, wszAboutMessage, L"About", MB_OK | MB_ICONINFORMATION );
                    break;
                }
            }
            return MENU_NOREDRAW;
        }
    }

    return MENU_ABSENT;
}

#endif // __MENU_HANDLER_INCLUDED__