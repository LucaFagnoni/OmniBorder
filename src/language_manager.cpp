#include "language_manager.h"
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <windows.h>

LanguageManager::LanguageManager()
{
    currentLanguage = DetectSystemLanguage();
    LoadTranslations(currentLanguage);
}

wxString LanguageManager::DetectSystemLanguage()
{
    LANGID langId = GetUserDefaultUILanguage();
    return (PRIMARYLANGID(langId) == LANG_SPANISH) ? "es" : "en";
}

wxString LanguageManager::ReadKey(const wxFileConfig& config, const wxString &sec, const wxString &k) const
{
    wxString v;
    const_cast<wxFileConfig&>(config).Read(sec + "/" + k, &v);
    return v.Trim(true).Trim(false);
}

void LanguageManager::LoadTranslations(const wxString &langCode)
{
    txt.clear();
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxString exeDir = wxFileName(exePath).GetPath();
    wxString cwd = wxFileName::GetCwd();
    
    wxArrayString possiblePaths;
    possiblePaths.Add(exeDir + "/languages/" + langCode + ".ini");
    possiblePaths.Add(exeDir + "/../languages/" + langCode + ".ini");
    possiblePaths.Add(cwd + "/languages/" + langCode + ".ini");
    possiblePaths.Add(cwd + "/../languages/" + langCode + ".ini");
    possiblePaths.Add("languages/" + langCode + ".ini");
    
    wxString iniPath;
    for (size_t i = 0; i < possiblePaths.GetCount(); i++)
    {
        if (wxFileExists(possiblePaths[i]))
        {
            iniPath = possiblePaths[i];
            break;
        }
    }

    if (iniPath.IsEmpty() || !wxFileExists(iniPath))
    {
        wxLogMessage("Language file not found, using defaults");
        txt["File"] = "File";
        txt["Exit"] = "Exit";
        txt["Options"] = "Options";
        txt["Language"] = "Language";
        txt["LangEs"] = "Spanish";
        txt["LangEn"] = "English";
        txt["StartWithWindows"] = "Start with Windows";
        txt["MinimizeToTray"] = "Minimize to Tray";
        txt["DarkMode"] = "Dark Mode";
        txt["ModifyHotkeys"] = "Configure Hotkeys";
        txt["Help"] = "Help";
        txt["TargetMonitor"] = "Target Monitor:";
        txt["CurrentMonitor"] = "Current Monitor";
        txt["HotkeyDlgTitle"] = "Configure System Hotkeys";
        txt["HkBorderless"] = "Toggle Borderless:";
        txt["HkMoveLeft"] = "Move Monitor Left:";
        txt["HkMoveRight"] = "Move Monitor Right:";
        txt["HotkeyHint"] = "Click a field and press your new keys combo.";
        txt["ShowInterface"] = "Show Interface";
        txt["About"] = "About";
        txt["SearchHint"] = "Filter windows...";
        txt["ThemeDark"] = "Dark Mode";
        txt["ThemeLight"] = "Light Mode";
        txt["ActiveApps"] = "Active Applications";
        txt["BorderlessActive"] = "Borderless Apps";
        txt["TechnicalDetails"] = "Technical Details";
        txt["HwndLabel"] = "Window HWND:";
        txt["ResolutionLabel"] = "Resolution:";
        txt["None"] = "None";
        txt["RestoreAll"] = "Restore All";
        txt["ScanSystem"] = "Scan System";
        txt["TrayTooltip"] = "OmniBorder";
        txt["AboutTitle"] = "About OmniBorder";
        txt["AboutText"] = "OmniBorder v1.0.0";
        txt["RestoreSuccess"] = "Restored %zu windows";
        txt["InfoTitle"] = "Information";
        return;
    }

    wxFileConfig config("", "", iniPath, "", wxCONFIG_USE_LOCAL_FILE);
    config.SetPath("/");

    txt["File"] = ReadKey(config, "Menu", "File");
    txt["Exit"] = ReadKey(config, "Menu", "Exit");
    txt["Options"] = ReadKey(config, "Menu", "Options");
    txt["Language"] = ReadKey(config, "Menu", "Language");
    txt["LangEs"] = ReadKey(config, "Menu", "LangEs");
    txt["LangEn"] = ReadKey(config, "Menu", "LangEn");
    txt["StartWithWindows"] = (langCode == "es") ? "Iniciar con Windows" : "Start with Windows";
    txt["ModifyHotkeys"] = ReadKey(config, "Menu", "ModifyHotkeys");
    txt["MinimizeToTray"] = ReadKey(config, "Menu", "MinimizeToTray");
    txt["DarkMode"] = ReadKey(config, "Menu", "DarkMode");
    if (txt["DarkMode"].IsEmpty())
    {
        txt["DarkMode"] = (langCode == "es") ? "Modo Oscuro" : "Dark Mode";
    }
    if (txt["MinimizeToTray"].IsEmpty())
    {
        txt["MinimizeToTray"] = (langCode == "es") ? "Minimizar a la Bandeja" : "Minimize to Tray";
    }
    txt["Help"] = ReadKey(config, "Menu", "Help");
    txt["About"] = ReadKey(config, "Menu", "About");
    txt["ShowInterface"] = ReadKey(config, "Menu", "ShowInterface");
    txt["SearchHint"] = ReadKey(config, "Labels", "SearchHint");
    txt["ThemeDark"] = ReadKey(config, "Labels", "ThemeDark");
    txt["ThemeLight"] = ReadKey(config, "Labels", "ThemeLight");
    txt["ActiveApps"] = ReadKey(config, "Labels", "ActiveApps");
    txt["BorderlessActive"] = ReadKey(config, "Labels", "BorderlessActive");
    txt["TechnicalDetails"] = ReadKey(config, "Labels", "TechnicalDetails");
    txt["HwndLabel"] = ReadKey(config, "Labels", "HwndLabel");
    txt["ResolutionLabel"] = ReadKey(config, "Labels", "ResolutionLabel");
    txt["None"] = ReadKey(config, "Labels", "None");
    txt["RestoreAll"] = ReadKey(config, "Labels", "RestoreAll");
    txt["ScanSystem"] = ReadKey(config, "Labels", "ScanSystem");
    txt["TrayTooltip"] = ReadKey(config, "Labels", "TrayTooltip");
    txt["TargetMonitor"] = ReadKey(config, "Labels", "TargetMonitor");
    txt["CurrentMonitor"] = ReadKey(config, "Labels", "CurrentMonitor");
    txt["AboutTitle"] = ReadKey(config, "Messages", "AboutTitle");
    txt["AboutText"] = ReadKey(config, "Messages", "AboutText");
    txt["RestoreSuccess"] = ReadKey(config, "Messages", "RestoreSuccess");
    txt["InfoTitle"] = ReadKey(config, "Messages", "InfoTitle");

    txt["HotkeyDlgTitle"] = ReadKey(config, "Hotkeys", "HotkeyDlgTitle");
    txt["HkBorderless"] = ReadKey(config, "Hotkeys", "HkBorderless");
    txt["HkMoveLeft"] = ReadKey(config, "Hotkeys", "HkMoveLeft");
    txt["HkMoveRight"] = ReadKey(config, "Hotkeys", "HkMoveRight");
    txt["HotkeyHint"] = ReadKey(config, "Hotkeys", "HotkeyHint");

    wxString aboutText = ReadKey(config, "Messages", "AboutText");
    if (!aboutText.IsEmpty()) aboutText.Replace("\\n", "\n");
    txt["AboutText"] = aboutText;
}

wxString LanguageManager::GetTranslation(const wxString& key, const wxString& fallback) const
{
    auto it = txt.find(key);
    if (it != txt.end() && !it->second.IsEmpty())
    {
        return it->second;
    }
    return fallback.IsEmpty() ? key : fallback;
}