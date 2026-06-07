#ifndef MAIN_H
#define MAIN_H

#include <wx/wx.h>
#include "types.h"
#include "window_manager.h"
#include "hotkey_manager.h"
#include "config_manager.h"
#include "language_manager.h"
#include "monitor_manager.h"
#include "tray_icon.h"

class MainWindow : public wxFrame
{
public:
    MainWindow();
    virtual ~MainWindow();

    void ShowMainWindow();
    void ToggleStartupOnWindows(bool enable);
    bool CheckStartupOnWindows();

    wxString GetTranslation(const wxString& key, const wxString& fallback = "");
    
    void ProcessGlobalBorderlessHotkey();
    void ProcessGlobalMoveHotkey(bool moveRight);
    void ApplyBorderlessMode(WindowInfo &info);
    
    bool IsDarkModeEnabled() const { return isDarkMode; }
    void ApplyDarkTheme(bool dark);

private:
    wxPanel *mainPanel;
    wxMenuBar *menuBar;
    wxMenu *menuFile;
    wxMenu *menuOptions;
    wxMenu *menuLanguageSub;
    wxMenu *menuHelp;

    wxMenuItem *menuItemStartup;
    wxMenuItem *menuItemHotkeys;
    wxMenuItem *menuItemLanguageSub;
    wxMenuItem *menuItemMinimizeToTray;
    wxMenuItem *menuItemDarkMode;

    wxStaticText *labelSearchBox;
    wxStaticText *labelNormalList;
    wxStaticText *labelBorderlessList;
    wxStaticText *labelDetailsBox;

    wxTextCtrl *textCtrlSearch;
    wxListBox *listBoxNormal;
    wxListBox *listBoxBorderless;

    wxStaticText *labelHwnd;
    wxStaticText *labelSize;
    wxStaticText *statusTextHwnd;
    wxStaticText *statusTextSize;
    
    wxStaticText *labelTargetMonitor;
    wxChoice *choiceMonitors;

    wxButton *buttonRefresh;
    wxButton *buttonToBorderless;
    wxButton *buttonToNormal;
    wxButton *buttonRestoreAll;
    wxButton *buttonTheme;

    TrayIcon *trayIcon;
    
    std::vector<size_t> filteredNormalIndices;
    std::vector<size_t> filteredBorderlessIndices;
    
    WindowManager windowManager;
    HotkeyManager hotkeyManager;
    ConfigManager configManager;
    LanguageManager languageManager;
    MonitorManager monitorManager;
    
    bool isDarkMode;
    bool minimizeToTray;
    
    void OnIconize(wxIconizeEvent &event);

    void CreateUI();
    void UpdateUITexts();
    void ApplyTheme();

    void FilterLists();

    void OnRefresh(wxCommandEvent &event);
    void OnMoveToBorderless(wxCommandEvent &event);
    void OnRestoreToNormal(wxCommandEvent &event);
    void OnRestoreAll(wxCommandEvent &event);
    void OnSearchChange(wxCommandEvent &event);
    void OnNormalListSelect(wxCommandEvent &event);
    void OnBorderlessListSelect(wxCommandEvent &event);
    void OnMonitorChoiceChange(wxCommandEvent &event);
    void OnToggleTheme(wxCommandEvent &event);
    void OnClose(wxCloseEvent &event);

    void OnMenuExit(wxCommandEvent &event);
    void OnMenuLanguageEs(wxCommandEvent &event);
    void OnMenuLanguageEn(wxCommandEvent &event);
    void OnMenuAbout(wxCommandEvent &event);
    void OnToggleStartup(wxCommandEvent &event);
    void OnOpenHotkeysDialog(wxCommandEvent &event);
    void OnToggleMinimizeToTray(wxCommandEvent &event);
    void OnToggleDarkMode(wxCommandEvent &event);

    void RestoreWindowMode(WindowInfo &info);
    void UpdateTechnicalDetails(HWND hwnd, int monitorIdx);
    int GetCurrentMonitorIndexOfWindow(HWND hwnd);

#ifdef __WXMSW__
    virtual WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam) override;
#endif

    DECLARE_EVENT_TABLE()
};

#endif