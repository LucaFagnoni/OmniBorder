#include "monitor_manager.h"

MonitorManager::MonitorManager()
{
    DetectarMonitores();
}

void MonitorManager::DetectarMonitores()
{
    detectedMonitors.clear();
    EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&detectedMonitors));
}

int MonitorManager::GetMonitorIndex(HMONITOR hMonitor) const
{
    for (size_t i = 0; i < detectedMonitors.size(); ++i)
    {
        if (detectedMonitors[i].hMonitor == hMonitor)
        {
            return static_cast<int>(i);
        }
    }
    return -1;
}

HMONITOR MonitorManager::GetMonitorByIndex(int index) const
{
    if (index >= 0 && index < static_cast<int>(detectedMonitors.size()))
    {
        return detectedMonitors[index].hMonitor;
    }
    return nullptr;
}

wxString MonitorManager::GetMonitorFriendlyNameCCD(LPCWSTR deviceName)
{
    UINT32 numPathArrayElements = 0;
    UINT32 numModeInfoArrayElements = 0;
    
    if (GetDisplayConfigBufferSizes(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &numModeInfoArrayElements) == ERROR_SUCCESS)
    {
        std::vector<DISPLAYCONFIG_PATH_INFO> pathArray(numPathArrayElements);
        std::vector<DISPLAYCONFIG_MODE_INFO> modeInfoArray(numModeInfoArrayElements);
        
        if (QueryDisplayConfig(QDC_ONLY_ACTIVE_PATHS, &numPathArrayElements, &pathArray[0], 
                               &numModeInfoArrayElements, &modeInfoArray[0], nullptr) == ERROR_SUCCESS)
        {
            for (UINT32 i = 0; i < numPathArrayElements; ++i)
            {
                DISPLAYCONFIG_TARGET_DEVICE_NAME targetName = {};
                targetName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_TARGET_NAME;
                targetName.header.size = sizeof(DISPLAYCONFIG_TARGET_DEVICE_NAME);
                targetName.header.adapterId = pathArray[i].targetInfo.adapterId;
                targetName.header.id = pathArray[i].targetInfo.id;
                
                if (DisplayConfigGetDeviceInfo(&targetName.header) == ERROR_SUCCESS)
                {
                    DISPLAYCONFIG_SOURCE_DEVICE_NAME sourceName = {};
                    sourceName.header.type = DISPLAYCONFIG_DEVICE_INFO_GET_SOURCE_NAME;
                    sourceName.header.size = sizeof(DISPLAYCONFIG_SOURCE_DEVICE_NAME);
                    sourceName.header.adapterId = pathArray[i].sourceInfo.adapterId;
                    sourceName.header.id = pathArray[i].sourceInfo.id;
                    
                    if (DisplayConfigGetDeviceInfo(&sourceName.header) == ERROR_SUCCESS)
                    {
                        if (wcscmp(sourceName.viewGdiDeviceName, deviceName) == 0)
                        {
                            if (targetName.flags.friendlyNameFromEdid && wcslen(targetName.monitorFriendlyDeviceName) > 0)
                            {
                                return wxString(targetName.monitorFriendlyDeviceName).Trim().Trim(false);
                            }
                        }
                    }
                }
            }
        }
    }
    return "";
}

BOOL CALLBACK MonitorManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
    std::vector<MonitorInfoData> *monitors = reinterpret_cast<std::vector<MonitorInfoData>*>(dwData);
    
    MONITORINFOEXW mi;
    mi.cbSize = sizeof(MONITORINFOEXW);
    if (GetMonitorInfoW(hMonitor, &mi))
    {
        wxString techName(mi.szDevice);
        wxString friendlyName = GetMonitorFriendlyNameCCD(mi.szDevice);

        if (friendlyName.IsEmpty())
        {
            DISPLAY_DEVICEW dd;
            dd.cb = sizeof(DISPLAY_DEVICEW);
            DWORD devIndex = 0;
            while (EnumDisplayDevicesW(mi.szDevice, devIndex, &dd, 0))
            {
                if (dd.StateFlags & DISPLAY_DEVICE_ACTIVE)
                {
                    friendlyName = wxString(dd.DeviceString).Trim().Trim(false);
                    break;
                }
                devIndex++;
            }
        }

        if (friendlyName.IsEmpty())
        {
            friendlyName = wxString::Format("Display %d", static_cast<int>(monitors->size()) + 1);
        }

        MonitorInfoData data = { hMonitor, techName, friendlyName, *lprcMonitor };
        monitors->push_back(data);
    }
    return TRUE;
}