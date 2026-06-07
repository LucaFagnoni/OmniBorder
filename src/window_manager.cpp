#include <wx/wx.h>
#include <wx/filename.h>
#include "window_manager.h"
#include <psapi.h>
#include <algorithm>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

WindowManager::WindowManager()
{
}

void WindowManager::EnumerateWindows()
{
    RemoveInvalidWindows();
    EnumWindows(EnumWindowsProc, reinterpret_cast<LPARAM>(this));
}

void WindowManager::RemoveInvalidWindows()
{
    trackedWindows.erase(
        std::remove_if(trackedWindows.begin(), trackedWindows.end(),
                       [](const WindowInfo &info)
                       { return !info.isCurrentlyBorderless && !IsWindow(info.hwnd); }),
        trackedWindows.end());
}

bool WindowManager::ShouldFilterWindow(HWND hwnd)
{
    if (!IsWindowVisible(hwnd)) return true;

    LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) return true;

    wchar_t title[256];
    if (GetWindowTextW(hwnd, title, 256) == 0) return true;
    wxString strTitle(title);
    if (strTitle.Trim().IsEmpty()) return true;

    DWORD processId;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0 || processId == GetCurrentProcessId()) return true;

    wxString exeName = GetExecutableName(hwnd);
    
    static const std::vector<std::wstring> filterList = {
        L"explorer.exe", L"taskmgr.exe", L"dwm.exe", 
        L"shellexperiencehost.exe", L"searchhost.exe", 
        L"applicationframehost.exe", L"nvidia share.exe", 
        L"discordhookhelper64.exe"
    };
    
    for (const auto& filtered : filterList)
    {
        if (exeName.Lower() == wxString(filtered).Lower())
        {
            return true;
        }
    }

    RECT r;
    if (GetWindowRect(hwnd, &r))
    {
        int w = r.right - r.left;
        int h = r.bottom - r.top;
        if (w < 150 || h < 150) return true;
    }

    return false;
}

wxString WindowManager::GetExecutableName(HWND hwnd)
{
    DWORD processId = 0;
    GetWindowThreadProcessId(hwnd, &processId);
    if (processId == 0) return "";

    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    if (!hProcess) return "";

    wchar_t buffer[MAX_PATH];
    if (GetModuleBaseNameW(hProcess, nullptr, buffer, MAX_PATH) == 0)
    {
        GetProcessImageFileNameW(hProcess, buffer, MAX_PATH);
    }
    CloseHandle(hProcess);

    wxString exePath(buffer);
    return wxFileName(exePath).GetFullName().Lower();
}

BOOL CALLBACK WindowManager::EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
    WindowManager *self = reinterpret_cast<WindowManager *>(lParam);
    
    if (self->ShouldFilterWindow(hwnd)) return TRUE;

    LONG style = GetWindowLongW(hwnd, GWL_STYLE);
    LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
    
    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    wxString strTitle(title);

    WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
    GetWindowPlacement(hwnd, &placement);

    wxString exeName = self->GetExecutableName(hwnd);

    auto it = std::find_if(self->trackedWindows.begin(), self->trackedWindows.end(),
                           [hwnd](const WindowInfo &info) { return info.hwnd == hwnd; });

    if (it == self->trackedWindows.end())
    {
        // New window - capture original state with initialization
        WindowInfo info = {hwnd, strTitle, exeName, style, exStyle, placement, {0, 0, 0, 0}, false, false, -1};
        self->trackedWindows.push_back(info);
    }
    else
    {
        // Existing window - ONLY update title, preserve original state capture
        // DO NOT overwrite originalStyle/originalExStyle/placement/originalRect to prevent corruption
        it->title = strTitle;
    }
    return TRUE;
}

void WindowManager::ApplyBorderlessMode(WindowInfo &info, const std::vector<MonitorInfoData>& monitors)
{
    HWND hwnd = info.hwnd;
    if (!hwnd || !IsWindow(hwnd)) return;

    // 1. PRE-GEOMETRY CAPTURE: Mandatory capture of original state before any modification
    // Only capture if state hasn't been captured yet (prevents corruption on repeated calls)
    if (!info.originalStateCaptured)
    {
        // Capture original styles (GWL_STYLE and GWL_EXSTYLE)
        info.originalStyle = GetWindowLongW(hwnd, GWL_STYLE);
        info.originalExStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
        
        // Capture original geometry using GetWindowPlacement (captures state: normal/maximized/minimized)
        info.placement.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hwnd, &info.placement);
        
        // Also capture explicit RECT for pixel-perfect restoration
        GetWindowRect(hwnd, &info.originalRect);
        
        // Mark state as captured to prevent future overwrites
        info.originalStateCaptured = true;
    }

    // 2. CORRUPT STATE CONTROL: Validate we have valid original data before proceeding
    // If original style lacks WS_CAPTION, window was already borderless-like - skip style changes
    bool wasAlreadyBorderlessLike = (info.originalStyle & WS_CAPTION) == 0;
    
    // 3. APPLY BORDERLESS STYLES (only if original window had caption/frame)
    if (!wasAlreadyBorderlessLike)
    {
        LONG currentStyle = GetWindowLongW(hwnd, GWL_STYLE);
        LONG currentExStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);

        // Remove window frame elements
        LONG newStyle = currentStyle;
        LONG newExStyle = currentExStyle;

        newStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_BORDER | WS_DLGFRAME | WS_VSCROLL | WS_HSCROLL);
        newStyle |= WS_POPUP;

        newExStyle &= ~(WS_EX_TOOLWINDOW | WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_DLGMODALFRAME);

        SetWindowLongW(hwnd, GWL_STYLE, newStyle);
        SetWindowLongW(hwnd, GWL_EXSTYLE, newExStyle);
    }

    // 4. CALCULATE TARGET MONITOR AND APPLY SIZE
    RECT targetRect;
    bool monitorValido = false;

    if (info.targetMonitorIndex >= 0 && info.targetMonitorIndex < static_cast<int>(monitors.size()))
    {
        targetRect = monitors[info.targetMonitorIndex].rect;
        monitorValido = true;
    }
    
    if (!monitorValido)
    {
        HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
        MONITORINFO mInfo = {sizeof(MONITORINFO)};
        GetMonitorInfoW(hMonitor, &mInfo);
        targetRect = mInfo.rcMonitor;
        
        // Find current monitor index
        for (int i = 0; i < static_cast<int>(monitors.size()); i++)
        {
            if (monitors[i].hMonitor == hMonitor)
            {
                info.targetMonitorIndex = i;
                break;
            }
        }
    }

    int width = targetRect.right - targetRect.left;
    int height = targetRect.bottom - targetRect.top;

    // 5. FORCE REDRAW WITH SWP_FRAMECHANGED
    // Remove SWP_NOACTIVATE to allow activation
    ::SetWindowPos(hwnd, HWND_TOPMOST,
                  targetRect.left,
                  targetRect.top,
                  width,
                  height,
                  SWP_SHOWWINDOW | SWP_FRAMECHANGED);

    // 6. AGGRESSIVELY RETAIN FOCUS - Critical for preventing window from going to background
    SetActiveWindow(hwnd);
    SetForegroundWindow(hwnd);

    // 7. DWM INTEGRATION: Disable modern title bar rendering for apps like Discord, Windows Terminal, UWP/XAML
    // DWMWA_NCRENDERING_POLICY=TRUE forces DWM to turn off custom non-client rendering
    BOOL dwmEnabled = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &dwmEnabled, sizeof(dwmEnabled));

    info.isCurrentlyBorderless = true;
}

void WindowManager::RestoreWindowMode(WindowInfo &info)
{
    HWND hwnd = info.hwnd;
    if (!hwnd || !IsWindow(hwnd)) return;

    // If never was borderless or state was never captured, nothing to restore
    if (!info.isCurrentlyBorderless || !info.originalStateCaptured) return;

    // 1. RESTORE ORIGINAL STYLES FIRST
    SetWindowLongW(hwnd, GWL_STYLE, info.originalStyle);
    SetWindowLongW(hwnd, GWL_EXSTYLE, info.originalExStyle);

    // 2. FORCE FRAME REDRAW (CRITICAL)
    // SWP_FRAMECHANGED forces Windows to recalculate title bar and frames
    // SWP_NOMOVE and SWP_NOSIZE keep current position temporarily
    ::SetWindowPos(hwnd, HWND_NOTOPMOST, 
                   0, 0, 0, 0, 
                   SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

    // 3. FAITHFUL RESTORATION: Use saved geometric data for pixel-perfect restoration
    // Method A: Restore using WINDOWPLACEMENT (preserves normal/maximized/minimized state)
    info.placement.length = sizeof(WINDOWPLACEMENT);
    
    // Safety check for placement
    if (info.placement.showCmd == SW_SHOWMINIMIZED)
    {
        // If minimized, restore to normal
        info.placement.showCmd = SW_SHOWNORMAL;
    }
    
    SetWindowPlacement(hwnd, &info.placement);

    // Method B: Alternative pixel-perfect restoration using originalRect
    // This ensures exact coordinate restoration if placement fails
    /*
    int width = info.originalRect.right - info.originalRect.left;
    int height = info.originalRect.bottom - info.originalRect.top;
    ::SetWindowPos(hwnd, HWND_NOTOPMOST,
                   info.originalRect.left,
                   info.originalRect.top,
                   width,
                   height,
                   SWP_SHOWWINDOW | SWP_FRAMECHANGED);
    */

    // 4. DWM INTEGRATION: Re-enable modern title bar rendering
    // DWMWA_NCRENDERING_POLICY=FALSE allows apps to render their own custom title bars again
    BOOL dwmEnabled = FALSE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &dwmEnabled, sizeof(dwmEnabled));

    // 5. MARK AS RESTORED (but preserve originalStateCaptured for potential future use)
    info.isCurrentlyBorderless = false;
}

void WindowManager::MoveBorderlessToMonitor(WindowInfo &info, int newMonitorIndex, const std::vector<MonitorInfoData>& monitors)
{
    HWND hwnd = info.hwnd;
    if (!hwnd || !IsWindow(hwnd)) return;
    if (!info.isCurrentlyBorderless) return;
    if (newMonitorIndex < 0 || newMonitorIndex >= static_cast<int>(monitors.size())) return;

    // Obtener las dimensiones del nuevo monitor
    RECT targetRect = monitors[newMonitorIndex].rect;
    int width = targetRect.right - targetRect.left;
    int height = targetRect.bottom - targetRect.top;

    // Mover y redimensionar la ventana al nuevo monitor
    // SWP_FRAMECHANGED es crítico para forzar el redibujado del marco
    ::SetWindowPos(hwnd, HWND_TOPMOST,
                  targetRect.left,
                  targetRect.top,
                  width,
                  height,
                  SWP_SHOWWINDOW | SWP_FRAMECHANGED | SWP_NOACTIVATE);

    // Actualizar el índice del monitor objetivo
    info.targetMonitorIndex = newMonitorIndex;
}

void WindowManager::FilterWindows(const wxString& query, 
                                  std::vector<size_t>& normalIndices, 
                                  std::vector<size_t>& borderlessIndices)
{
    normalIndices.clear();
    borderlessIndices.clear();
    
    wxString queryLower = query.Lower();

    for (size_t i = 0; i < trackedWindows.size(); ++i)
    {
        if (!IsWindow(trackedWindows[i].hwnd)) continue;
        
        if (query.IsEmpty() || trackedWindows[i].title.Lower().Contains(queryLower))
        {
            if (!trackedWindows[i].isCurrentlyBorderless)
            {
                normalIndices.push_back(i);
            }
            else
            {
                borderlessIndices.push_back(i);
            }
        }
    }
}