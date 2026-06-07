#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <wx/tokenzr.h>
#include "config_manager.h"

ConfigManager::ConfigManager()
{
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    configPath = wxFileName(exePath).GetPath() + "/config.ini";
}

void ConfigManager::LoadPersistentConfig(HotkeyManager& hotkeyManager, bool& minimizeToTray, bool& isDarkMode)
{
    if (!wxFileExists(configPath)) return;

    wxFileConfig config("", "", configPath, "", wxCONFIG_USE_LOCAL_FILE);
    
    wxString rawApps;
    config.Read("Persistence/BorderlessApps", &rawApps);

    wxStringTokenizer tokenizer(rawApps, ";");
    while (tokenizer.HasMoreTokens())
    {
        wxString token = tokenizer.GetNextToken().Trim();
        if (!token.IsEmpty())
        {
            if (token.Contains(":"))
            {
                wxString exe = token.BeforeFirst(':').Lower();
                long mIdx = -1;
                token.AfterFirst(':').ToLong(&mIdx);
                persistentBorderlessApps[exe] = static_cast<int>(mIdx);
            }
            else
            {
                persistentBorderlessApps[token.Lower()] = -1;
            }
        }
    }

    // Load Minimize to Tray setting (default: true)
    config.Read("Options/MinimizeToTray", &minimizeToTray, true);
    
    // Load Dark Mode setting (default: false)
    config.Read("Options/DarkMode", &isDarkMode, false);

    int borderlessMods = MOD_CONTROL | MOD_ALT;
    int borderlessVk = 'B';
    int moveLeftMods = MOD_CONTROL | MOD_ALT;
    int moveLeftVk = VK_LEFT;
    int moveRightMods = MOD_CONTROL | MOD_ALT;
    int moveRightVk = VK_RIGHT;

    config.Read("Hotkeys/Borderless_Mods", &borderlessMods, borderlessMods);
    config.Read("Hotkeys/Borderless_VK",   &borderlessVk, borderlessVk);
    config.Read("Hotkeys/MoveLeft_Mods",   &moveLeftMods, moveLeftMods);
    config.Read("Hotkeys/MoveLeft_VK",     &moveLeftVk, moveLeftVk);
    config.Read("Hotkeys/MoveRight_Mods",  &moveRightMods, moveRightMods);
    config.Read("Hotkeys/MoveRight_VK",    &moveRightVk, moveRightVk);
    
    hotkeyManager.SetBorderlessHotkey({borderlessMods, borderlessVk});
    hotkeyManager.SetMoveLeftHotkey({moveLeftMods, moveLeftVk});
    hotkeyManager.SetMoveRightHotkey({moveRightMods, moveRightVk});
}

void ConfigManager::SavePersistentConfig(const HotkeyManager& hotkeyManager, bool minimizeToTray, bool isDarkMode)
{
    wxFileConfig config("", "", configPath, "", wxCONFIG_USE_LOCAL_FILE);
    
    wxString serializedApps;
    for (const auto &pair : persistentBorderlessApps)
    {
        serializedApps += pair.first + ":" + wxString::Format("%d", pair.second) + ";";
    }
    config.Write("Persistence/BorderlessApps", serializedApps);
    
    config.Write("Options/MinimizeToTray", minimizeToTray);
    config.Write("Options/DarkMode", isDarkMode);
    
    const HotkeyConfig& hkBorderless = hotkeyManager.GetBorderlessHotkey();
    const HotkeyConfig& hkMoveLeft = hotkeyManager.GetMoveLeftHotkey();
    const HotkeyConfig& hkMoveRight = hotkeyManager.GetMoveRightHotkey();
    
    config.Write("Hotkeys/Borderless_Mods", hkBorderless.modifiers);
    config.Write("Hotkeys/Borderless_VK", hkBorderless.vkCode);
    config.Write("Hotkeys/MoveLeft_Mods", hkMoveLeft.modifiers);
    config.Write("Hotkeys/MoveLeft_VK", hkMoveLeft.vkCode);
    config.Write("Hotkeys/MoveRight_Mods", hkMoveRight.modifiers);
    config.Write("Hotkeys/MoveRight_VK", hkMoveRight.vkCode);
}