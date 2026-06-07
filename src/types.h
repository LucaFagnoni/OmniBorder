#ifndef TYPES_H
#define TYPES_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <wx/wx.h>
#include <string>
#include <vector>
#include <map>

struct MonitorInfoData
{
    HMONITOR hMonitor;
    wxString technicalName; 
    wxString friendlyName;  
    RECT rect;
};

struct WindowInfo
{
    HWND hwnd;
    wxString title;
    wxString exeName;
    LONG originalStyle;
    LONG originalExStyle;
    WINDOWPLACEMENT placement;
    RECT originalRect;  // Original window rectangle before borderless
    bool isCurrentlyBorderless;
    bool originalStateCaptured;  // Flag to prevent state corruption
    int targetMonitorIndex; 
};

struct HotkeyConfig
{
    int modifiers;
    int vkCode;
};

enum ControlIds
{
    ID_REFRESH_BTN = wxID_HIGHEST + 1,
    ID_MOVE_TO_BORDERLESS_BTN,
    ID_RESTORE_TO_NORMAL_BTN,
    ID_RESTORE_ALL_BTN,
    ID_NORMAL_LIST,
    ID_BORDERLESS_LIST,
    ID_SEARCH_BOX,
    ID_THEME_BTN,
    ID_MONITOR_CHOICE,
    ID_MENU_EXIT,
    ID_MENU_LANG_ES,
    ID_MENU_LANG_EN,
    ID_MENU_ABOUT,
    ID_TRAY_SHOW_INTERFACE,
    ID_MENU_STARTUP,
    ID_MENU_HOTKEYS,
    ID_MENU_MINIMIZE_TO_TRAY,
    ID_MENU_DARK_MODE,
    
    HOTKEY_BORDERLESS_ID = 9001,
    HOTKEY_MOVE_LEFT_ID  = 9002,
    HOTKEY_MOVE_RIGHT_ID = 9003
};

#endif