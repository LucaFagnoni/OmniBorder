#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "types.h"
#include "hotkey_manager.h"
#include <wx/wx.h>
#include <map>

class ConfigManager
{
public:
    ConfigManager();
    
    void LoadPersistentConfig(HotkeyManager& hotkeyManager, bool& minimizeToTray, bool& isDarkMode);
    void SavePersistentConfig(const HotkeyManager& hotkeyManager, bool minimizeToTray, bool isDarkMode);
    
    const std::map<wxString, int>& GetPersistentBorderlessApps() const { return persistentBorderlessApps; }
    std::map<wxString, int>& GetPersistentBorderlessApps() { return persistentBorderlessApps; }
    
    wxString GetConfigPath() const { return configPath; }
    
private:
    std::map<wxString, int> persistentBorderlessApps;
    wxString configPath;
};

#endif