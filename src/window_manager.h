#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include "types.h"
#include "monitor_manager.h"
#include <vector>
#include <functional>

class WindowManager
{
public:
    WindowManager();
    
    void EnumerateWindows();
    void ApplyBorderlessMode(WindowInfo &info, const std::vector<MonitorInfoData>& monitors);
    void RestoreWindowMode(WindowInfo &info);
    void MoveBorderlessToMonitor(WindowInfo &info, int newMonitorIndex, const std::vector<MonitorInfoData>& monitors);
    
    const std::vector<WindowInfo>& GetWindows() const { return trackedWindows; }
    std::vector<WindowInfo>& GetWindows() { return trackedWindows; }
    
    void FilterWindows(const wxString& query, 
                       std::vector<size_t>& normalIndices, 
                       std::vector<size_t>& borderlessIndices);
    
    wxString GetExecutableName(HWND hwnd);
    
    void RemoveInvalidWindows();
    
private:
    std::vector<WindowInfo> trackedWindows;
    
    static BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
    
    bool ShouldFilterWindow(HWND hwnd);
};

#endif