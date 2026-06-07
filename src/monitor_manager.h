#ifndef MONITOR_MANAGER_H
#define MONITOR_MANAGER_H

#include "types.h"
#include <vector>

class MonitorManager
{
public:
    MonitorManager();
    
    void DetectarMonitores();
    const std::vector<MonitorInfoData>& GetMonitors() const { return detectedMonitors; }
    int GetMonitorIndex(HMONITOR hMonitor) const;
    HMONITOR GetMonitorByIndex(int index) const;
    static wxString GetMonitorFriendlyNameCCD(LPCWSTR deviceName);
    
private:
    std::vector<MonitorInfoData> detectedMonitors;
    
    static BOOL CALLBACK MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
};

#endif