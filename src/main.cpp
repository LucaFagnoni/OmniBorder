#include "main.h"
#include "tray_icon.h"
#include "logger.h"
#include <wx/stdpaths.h>
#include <wx/fileconf.h>
#include <shellapi.h>
#include <functional>
#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
    EVT_BUTTON(ID_REFRESH_BTN, MainWindow::OnRefresh)
    EVT_BUTTON(ID_MOVE_TO_BORDERLESS_BTN, MainWindow::OnMoveToBorderless)
    EVT_BUTTON(ID_RESTORE_TO_NORMAL_BTN, MainWindow::OnRestoreToNormal)
    EVT_BUTTON(ID_RESTORE_ALL_BTN, MainWindow::OnRestoreAll)
    EVT_BUTTON(ID_THEME_BTN, MainWindow::OnToggleTheme)
    EVT_TEXT(ID_SEARCH_BOX, MainWindow::OnSearchChange)
    EVT_LISTBOX(ID_NORMAL_LIST, MainWindow::OnNormalListSelect)
    EVT_LISTBOX(ID_BORDERLESS_LIST, MainWindow::OnBorderlessListSelect)
    EVT_CHOICE(ID_MONITOR_CHOICE, MainWindow::OnMonitorChoiceChange)
    EVT_CLOSE(MainWindow::OnClose)
    EVT_MENU(ID_MENU_EXIT, MainWindow::OnMenuExit)
    EVT_MENU(ID_MENU_LANG_ES, MainWindow::OnMenuLanguageEs)
    EVT_MENU(ID_MENU_LANG_EN, MainWindow::OnMenuLanguageEn)
    EVT_MENU(ID_MENU_ABOUT, MainWindow::OnMenuAbout)
    EVT_MENU(ID_MENU_STARTUP, MainWindow::OnToggleStartup)
    EVT_MENU(ID_MENU_HOTKEYS, MainWindow::OnOpenHotkeysDialog)
    EVT_MENU(ID_MENU_MINIMIZE_TO_TRAY, MainWindow::OnToggleMinimizeToTray)
    EVT_MENU(ID_MENU_DARK_MODE, MainWindow::OnToggleDarkMode)
    EVT_ICONIZE(MainWindow::OnIconize)
wxEND_EVENT_TABLE()

MainWindow::MainWindow()
    : wxFrame(nullptr, wxID_ANY, "OmniBorder",
              wxDefaultPosition, wxSize(950, 740),
              wxDEFAULT_FRAME_STYLE & ~(wxRESIZE_BORDER | wxMAXIMIZE_BOX))
{
    LOG_INFO("MainWindow constructor started");
    
    isDarkMode = false;
    minimizeToTray = true;
    menuItemStartup = nullptr;
    menuItemHotkeys = nullptr;
    menuItemLanguageSub = nullptr;
    menuItemMinimizeToTray = nullptr;
    menuItemDarkMode = nullptr;
    LOG_INFO("Variables initialized");

    try
    {
        // Set window icon
        wxIcon appIcon;
        if (appIcon.LoadFile("aaaa_MAIN_ICON", wxBITMAP_TYPE_ICO_RESOURCE))
        {
            this->SetIcon(appIcon);
            LOG_INFO("Window icon set successfully");
        }
        else
        {
            LOG_INFO("Could not load window icon, using default");
        }

        trayIcon = new TrayIcon(this);
        LOG_INFO("TrayIcon created");
        
        this->Bind(wxEVT_ICONIZE, &MainWindow::OnIconize, this);
        LOG_INFO("Iconize event bound");

        LOG_INFO("Loading persistent config...");
        configManager.LoadPersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
        LOG_INFO(wxString::Format("Config loaded - isDarkMode=%d, minimizeToTray=%d", isDarkMode, minimizeToTray));
        
        LOG_INFO("Creating UI...");
        CreateUI();
        LOG_INFO("UI created");
        
        LOG_INFO("Updating UI texts...");
        UpdateUITexts();
        LOG_INFO("UI texts updated");
        
        // Apply theme colors based on dark mode setting
        LOG_INFO(wxString::Format("Applying theme (dark mode=%d)", isDarkMode));
        ApplyTheme();
        LOG_INFO("Theme applied");
        
        if (!minimizeToTray)
        {
            LOG_INFO("Hiding tray icon");
            trayIcon->HideTrayIcon();
        }
        
        LOG_INFO("Registering hotkeys...");
        hotkeyManager.RegisterHotkeys((HWND)this->GetHWND());
        LOG_INFO("Hotkeys registered");
        
        LOG_INFO("Centering window...");
        CentreOnScreen();
        LOG_INFO("Window centered");
        
        LOG_INFO("MainWindow constructor completed successfully");
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(wxString::Format("Exception in constructor: %s", e.what()));
        throw;
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception in constructor");
        throw;
    }
}

MainWindow::~MainWindow()
{
    hotkeyManager.UnregisterHotkeys((HWND)this->GetHWND());
    delete trayIcon;
}

#ifdef __WXMSW__
WXLRESULT MainWindow::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    if (message == WM_HOTKEY)
    {
        int hotkeyId = static_cast<int>(wParam);
        if (hotkeyManager.IsBorderlessHotkey(hotkeyId))
        {
            ProcessGlobalBorderlessHotkey();
            return 0;
        }
        else if (hotkeyManager.IsMoveLeftHotkey(hotkeyId))
        {
            ProcessGlobalMoveHotkey(false);
            return 0;
        }
        else if (hotkeyManager.IsMoveRightHotkey(hotkeyId))
        {
            ProcessGlobalMoveHotkey(true);
            return 0;
        }
    }
    return wxFrame::MSWWindowProc(message, wParam, lParam);
}
#endif

void MainWindow::ProcessGlobalBorderlessHotkey()
{
    HWND activeHwnd = GetForegroundWindow();
    if (!activeHwnd || activeHwnd == (HWND)this->GetHWND()) return;

    auto& windows = windowManager.GetWindows();
    auto it = std::find_if(windows.begin(), windows.end(),
                           [activeHwnd](const WindowInfo &info) { return info.hwnd == activeHwnd; });

    if (it != windows.end())
    {
        if (it->isCurrentlyBorderless)
        {
            RestoreWindowMode(*it);
            auto& persistentApps = configManager.GetPersistentBorderlessApps();
            if (persistentApps.count(it->exeName))
            {
                persistentApps.erase(it->exeName);
                configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
            }
        }
        else
        {
            ApplyBorderlessMode(*it);
            if (!it->exeName.IsEmpty())
            {
                auto& persistentApps = configManager.GetPersistentBorderlessApps();
                persistentApps[it->exeName] = it->targetMonitorIndex;
                configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
            }
        }
    }
    else
    {
        wchar_t title[256];
        GetWindowTextW(activeHwnd, title, 256);
        LONG style = GetWindowLongW(activeHwnd, GWL_STYLE);
        LONG exStyle = GetWindowLongW(activeHwnd, GWL_EXSTYLE);
        WINDOWPLACEMENT placement = {sizeof(WINDOWPLACEMENT)};
        GetWindowPlacement(activeHwnd, &placement);
        wxString exeName = windowManager.GetExecutableName(activeHwnd);

        WindowInfo info = {activeHwnd, wxString(title), exeName, style, exStyle, placement, {0,0,0,0}, false, false, -1};
        ApplyBorderlessMode(info);
        
        if (!info.exeName.IsEmpty())
        {
            auto& persistentApps = configManager.GetPersistentBorderlessApps();
            persistentApps[info.exeName] = -1;
            configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
        }
        windows.push_back(info);
    }

    FilterLists();
}

void MainWindow::ProcessGlobalMoveHotkey(bool moveRight)
{
    HWND activeHwnd = GetForegroundWindow();
    if (!activeHwnd || activeHwnd == (HWND)this->GetHWND()) return;

    int currentMonIdx = GetCurrentMonitorIndexOfWindow(activeHwnd);
    if (currentMonIdx == -1 || monitorManager.GetMonitors().empty()) return;

    int nextMonIdx = currentMonIdx + (moveRight ? 1 : -1);
    if (nextMonIdx >= static_cast<int>(monitorManager.GetMonitors().size())) nextMonIdx = 0;
    if (nextMonIdx < 0) nextMonIdx = static_cast<int>(monitorManager.GetMonitors().size()) - 1;

    auto& windows = windowManager.GetWindows();
    auto it = std::find_if(windows.begin(), windows.end(),
                           [activeHwnd](const WindowInfo &info) { return info.hwnd == activeHwnd; });

    if (it != windows.end())
    {
        it->targetMonitorIndex = nextMonIdx;
        if (it->isCurrentlyBorderless)
        {
            ApplyBorderlessMode(*it);
        }
        else
        {
            RECT currentRect;
            if (GetWindowRect(activeHwnd, &currentRect))
            {
                int w = currentRect.right - currentRect.left;
                int h = currentRect.bottom - currentRect.top;
                RECT mRect = monitorManager.GetMonitors()[nextMonIdx].rect;
                ::SetWindowPos(activeHwnd, nullptr, mRect.left + 50, mRect.top + 50, w, h, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
            }
        }
    }
    else
    {
        RECT currentRect;
        if (GetWindowRect(activeHwnd, &currentRect))
        {
            int w = currentRect.right - currentRect.left;
            int h = currentRect.bottom - currentRect.top;
            RECT mRect = monitorManager.GetMonitors()[nextMonIdx].rect;
            ::SetWindowPos(activeHwnd, nullptr, mRect.left + 50, mRect.top + 50, w, h, SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
        }
    }

    FilterLists();
}

void MainWindow::OnIconize(wxIconizeEvent &event)
{
    if (event.IsIconized())
    {
        if (minimizeToTray)
        {
            this->Show(false);
        }
        else
        {
            event.Skip();
        }
    }
    else
    {
        if (!minimizeToTray && trayIcon)
        {
            trayIcon->ShowTrayIcon();
        }
        event.Skip();
    }
}

void MainWindow::ShowMainWindow()
{
    if (!this->IsShown()) this->Show(true);
    if (this->IsIconized()) this->Iconize(false);
    this->Raise();
}

void MainWindow::OnClose(wxCloseEvent &event)
{
    if (event.CanVeto())
    {
        event.Veto();
        this->Show(false);
    }
    else
    {
        this->Destroy();
    }
}

void MainWindow::ToggleStartupOnWindows(bool enable)
{
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_WRITE, &hKey);
    if (lRes == ERROR_SUCCESS)
    {
        if (enable)
        {
            wxString exePath = wxStandardPaths::Get().GetExecutablePath();
            std::wstring wpath = exePath.ToStdWstring();
            RegSetValueExW(hKey, L"OmniBorder", 0, REG_SZ, (BYTE *)wpath.c_str(), (wpath.size() + 1) * sizeof(wchar_t));
        }
        else
        {
            RegDeleteValueW(hKey, L"OmniBorder");
        }
        RegCloseKey(hKey);
    }
}

bool MainWindow::CheckStartupOnWindows() 
{
    HKEY hKey;
    LONG lRes = RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Microsoft\\Windows\\CurrentVersion\\Run", 0, KEY_READ, &hKey);
    bool exists = false;
    if (lRes == ERROR_SUCCESS) {
        lRes = RegQueryValueExW(hKey, L"OmniBorder", nullptr, nullptr, nullptr, nullptr);
        if (lRes == ERROR_SUCCESS) exists = true;
        RegCloseKey(hKey);
    }
    return exists;
}

wxString MainWindow::GetTranslation(const wxString& key, const wxString& fallback)
{
    return languageManager.GetTranslation(key, fallback);
}

int MainWindow::GetCurrentMonitorIndexOfWindow(HWND hwnd)
{
    if (!hwnd || !IsWindow(hwnd)) return -1;
    HMONITOR hMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONEAREST);
    return monitorManager.GetMonitorIndex(hMonitor);
}

void MainWindow::ApplyBorderlessMode(WindowInfo &info)
{
    windowManager.ApplyBorderlessMode(info, monitorManager.GetMonitors());
}

void MainWindow::RestoreWindowMode(WindowInfo &info)
{
    windowManager.RestoreWindowMode(info);
}

void MainWindow::FilterLists()
{
    wxString query = textCtrlSearch->GetValue();
    windowManager.FilterWindows(query, filteredNormalIndices, filteredBorderlessIndices);
    
    listBoxNormal->Clear();
    listBoxBorderless->Clear();
    
    const auto& windows = windowManager.GetWindows();
    
    for (size_t idx : filteredNormalIndices)
    {
        listBoxNormal->Append(windows[idx].title);
    }
    
    for (size_t idx : filteredBorderlessIndices)
    {
        listBoxBorderless->Append(windows[idx].title);
    }
    
    statusTextHwnd->SetLabel(GetTranslation("None", "None"));
    statusTextSize->SetLabel("0 x 0");
    choiceMonitors->SetSelection(0);
}

void MainWindow::UpdateTechnicalDetails(HWND hwnd, int monitorIdx)
{
    if (hwnd && IsWindow(hwnd))
    {
        statusTextHwnd->SetLabel(wxString::Format("%p", (void *)hwnd));
        RECT rect;
        if (GetWindowRect(hwnd, &rect))
        {
            statusTextSize->SetLabel(wxString::Format("%d x %d", rect.right - rect.left, rect.bottom - rect.top));
        }

        int activeMonitorIndex = monitorIdx;
        if (activeMonitorIndex == -1)
        {
            activeMonitorIndex = GetCurrentMonitorIndexOfWindow(hwnd);
        }

        choiceMonitors->SetSelection(activeMonitorIndex + 1);
    }
}

void MainWindow::OnNormalListSelect(wxCommandEvent &WXUNUSED(e))
{
    int s = listBoxNormal->GetSelection();
    if (s != wxNOT_FOUND)
    {
        listBoxBorderless->DeselectAll();
        size_t idx = filteredNormalIndices[s];
        UpdateTechnicalDetails(windowManager.GetWindows()[idx].hwnd, 
                               windowManager.GetWindows()[idx].targetMonitorIndex);
    }
}

void MainWindow::OnBorderlessListSelect(wxCommandEvent &WXUNUSED(e))
{
    int s = listBoxBorderless->GetSelection();
    if (s != wxNOT_FOUND)
    {
        listBoxNormal->DeselectAll();
        size_t idx = filteredBorderlessIndices[s];
        UpdateTechnicalDetails(windowManager.GetWindows()[idx].hwnd, 
                               windowManager.GetWindows()[idx].targetMonitorIndex);
    }
}

void MainWindow::OnMonitorChoiceChange(wxCommandEvent &event)
{
    int sNormal = listBoxNormal->GetSelection();
    int sBorder = listBoxBorderless->GetSelection();
    size_t idx = 0;
    bool found = false;

    if (sNormal != wxNOT_FOUND) { idx = filteredNormalIndices[sNormal]; found = true; }
    else if (sBorder != wxNOT_FOUND) { idx = filteredBorderlessIndices[sBorder]; found = true; }

    if (found)
    {
        int selection = event.GetSelection();
        windowManager.GetWindows()[idx].targetMonitorIndex = selection - 1;

        auto& persistentApps = configManager.GetPersistentBorderlessApps();
        if (!windowManager.GetWindows()[idx].exeName.IsEmpty() && 
            persistentApps.count(windowManager.GetWindows()[idx].exeName))
        {
            persistentApps[windowManager.GetWindows()[idx].exeName] = windowManager.GetWindows()[idx].targetMonitorIndex;
            configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
        }

        if (IsWindow(windowManager.GetWindows()[idx].hwnd))
        {
            if (windowManager.GetWindows()[idx].isCurrentlyBorderless)
            {
                ApplyBorderlessMode(windowManager.GetWindows()[idx]);
            }
            else if (windowManager.GetWindows()[idx].targetMonitorIndex >= 0 && 
                     windowManager.GetWindows()[idx].targetMonitorIndex < static_cast<int>(monitorManager.GetMonitors().size()))
            {
                RECT currentRect;
                if (GetWindowRect(windowManager.GetWindows()[idx].hwnd, &currentRect))
                {
                    int w = currentRect.right - currentRect.left;
                    int h = currentRect.bottom - currentRect.top;
                    RECT mRect = monitorManager.GetMonitors()[windowManager.GetWindows()[idx].targetMonitorIndex].rect;
                    
                    ::SetWindowPos(windowManager.GetWindows()[idx].hwnd, nullptr, 
                                 mRect.left + 50, mRect.top + 50, w, h, 
                                 SWP_NOZORDER | SWP_NOSIZE | SWP_SHOWWINDOW);
                }
            }
            UpdateTechnicalDetails(windowManager.GetWindows()[idx].hwnd, 
                                   windowManager.GetWindows()[idx].targetMonitorIndex);
        }
    }
}

void MainWindow::OnSearchChange(wxCommandEvent &WXUNUSED(e)) 
{ 
    FilterLists(); 
}

void MainWindow::OnRefresh(wxCommandEvent &WXUNUSED(e))
{
    textCtrlSearch->Clear();
    windowManager.EnumerateWindows();
    FilterLists();
}

void MainWindow::OnMoveToBorderless(wxCommandEvent &WXUNUSED(e))
{
    int s = listBoxNormal->GetSelection();
    if (s == wxNOT_FOUND) return;
    size_t idx = filteredNormalIndices[s];
    
    ApplyBorderlessMode(windowManager.GetWindows()[idx]);

    if (!windowManager.GetWindows()[idx].exeName.IsEmpty())
    {
        auto& persistentApps = configManager.GetPersistentBorderlessApps();
        persistentApps[windowManager.GetWindows()[idx].exeName] = windowManager.GetWindows()[idx].targetMonitorIndex;
        configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    }
    FilterLists();
}

void MainWindow::OnRestoreToNormal(wxCommandEvent &WXUNUSED(e))
{
    int s = listBoxBorderless->GetSelection();
    if (s == wxNOT_FOUND) return;
    size_t idx = filteredBorderlessIndices[s];
    RestoreWindowMode(windowManager.GetWindows()[idx]);

    auto& persistentApps = configManager.GetPersistentBorderlessApps();
    if (persistentApps.count(windowManager.GetWindows()[idx].exeName))
    {
        persistentApps.erase(windowManager.GetWindows()[idx].exeName);
        configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    }
    FilterLists();
}

void MainWindow::OnRestoreAll(wxCommandEvent &WXUNUSED(e))
{
    size_t count = 0;
    auto& windows = windowManager.GetWindows();
    for (auto &info : windows)
    {
        if (info.isCurrentlyBorderless && IsWindow(info.hwnd))
        {
            RestoreWindowMode(info);
            count++;
        }
    }
    configManager.GetPersistentBorderlessApps().clear();
    configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    FilterLists();
    if (count > 0)
    {
        wxString msg = wxString::Format(GetTranslation("RestoreSuccess", "Restored %zu windows successfully."), count);
        wxMessageBox(msg, GetTranslation("InfoTitle", "Information"), wxOK | wxICON_INFORMATION, this);
    }
}

void MainWindow::OnToggleTheme(wxCommandEvent &WXUNUSED(event))
{
    isDarkMode = !isDarkMode;
    UpdateUITexts();
    ApplyTheme();
}

void MainWindow::OnMenuExit(wxCommandEvent &WXUNUSED(event)) 
{ 
    this->Destroy(); 
}

void MainWindow::OnMenuLanguageEs(wxCommandEvent &WXUNUSED(event))
{
    if (languageManager.GetCurrentLanguage() != "es")
    {
        languageManager.SetCurrentLanguage("es");
        languageManager.LoadTranslations("es");
        UpdateUITexts();
    }
}

void MainWindow::OnMenuLanguageEn(wxCommandEvent &WXUNUSED(event))
{
    if (languageManager.GetCurrentLanguage() != "en")
    {
        languageManager.SetCurrentLanguage("en");
        languageManager.LoadTranslations("en");
        UpdateUITexts();
    }
}

void MainWindow::OnMenuAbout(wxCommandEvent &WXUNUSED(event)) 
{ 
    wxMessageBox(GetTranslation("AboutText", "OmniBorder"), 
                 GetTranslation("AboutTitle", "About"), 
                 wxOK | wxICON_INFORMATION, this); 
}

void MainWindow::OnToggleStartup(wxCommandEvent &event) 
{
    bool shouldEnable = event.IsChecked();
    ToggleStartupOnWindows(shouldEnable);
}

void MainWindow::OnToggleMinimizeToTray(wxCommandEvent &event)
{
    minimizeToTray = event.IsChecked();
    
    if (minimizeToTray)
    {
        trayIcon->ShowTrayIcon();
    }
    else
    {
        trayIcon->HideTrayIcon();
    }
    
    configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
}

void MainWindow::OnToggleDarkMode(wxCommandEvent &event)
{
    isDarkMode = event.IsChecked();
    
    configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    ApplyDarkTheme(isDarkMode);
    ApplyTheme();
}

void MainWindow::ApplyDarkTheme(bool dark)
{
    HWND hwnd = (HWND)this->GetHWND();
    if (!hwnd) return;
    if (!IsWindow(hwnd)) return;
    
    BOOL useDark = dark ? TRUE : FALSE;
    ::DwmSetWindowAttribute(hwnd, 20, &useDark, sizeof(useDark));
}

void MainWindow::OnOpenHotkeysDialog(wxCommandEvent &WXUNUSED(event))
{
    hotkeyManager.UnregisterHotkeys((HWND)this->GetHWND());
    
    configManager.LoadPersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    
    wxDialog dlg(&dynamic_cast<wxFrame&>(*this), wxID_ANY, GetTranslation("HotkeyDlgTitle", "Configure Hotkeys"), 
                 wxDefaultPosition, wxSize(430, 260));
    
    wxBoxSizer* mainSizer = new wxBoxSizer(wxVERTICAL);
    wxFlexGridSizer* gridSizer = new wxFlexGridSizer(3, 2, 10, 10);
    gridSizer->AddGrowableCol(1, 1);

    HotkeyConfig hkBorderless = hotkeyManager.GetBorderlessHotkey();
    HotkeyConfig hkMoveLeft = hotkeyManager.GetMoveLeftHotkey();
    HotkeyConfig hkMoveRight = hotkeyManager.GetMoveRightHotkey();

    auto getHotkeyString = [](const HotkeyConfig& cfg) -> wxString {
        wxString str;
        if (cfg.modifiers & MOD_CONTROL) str += "Ctrl + ";
        if (cfg.modifiers & MOD_ALT)     str += "Alt + ";
        if (cfg.modifiers & MOD_SHIFT)   str += "Shift + ";
        if (cfg.modifiers & MOD_WIN)     str += "Win + ";

        if (cfg.vkCode == VK_LBUTTON) str += "Left Click";
        else if (cfg.vkCode == VK_RBUTTON) str += "Right Click";
        else if (cfg.vkCode == VK_MBUTTON) str += "Middle Click";
        else if (cfg.vkCode == VK_XBUTTON1) str += "Mouse X1";
        else if (cfg.vkCode == VK_XBUTTON2) str += "Mouse X2";
        else if (cfg.vkCode >= 'A' && cfg.vkCode <= 'Z') str += (char)cfg.vkCode;
        else if (cfg.vkCode == VK_LEFT)  str += "Left Arrow";
        else if (cfg.vkCode == VK_RIGHT) str += "Right Arrow";
        else if (cfg.vkCode == VK_UP)    str += "Up Arrow";
        else if (cfg.vkCode == VK_DOWN)  str += "Down Arrow";
        else if (cfg.vkCode >= VK_F1 && cfg.vkCode <= VK_F12) str += wxString::Format("F%d", cfg.vkCode - VK_F1 + 1);
        else if (cfg.vkCode >= VK_F13 && cfg.vkCode <= VK_F24) str += wxString::Format("F%d", cfg.vkCode - VK_F13 + 13);
        else str += wxString::Format("Key (%d)", cfg.vkCode);

        return str;
    };

    wxStaticText* lblB = new wxStaticText(&dlg, wxID_ANY, GetTranslation("HkBorderless", "Toggle Borderless:"));
    wxStaticText* textCtrlBorderless = new wxStaticText(&dlg, wxID_ANY, getHotkeyString(hkBorderless), 
                                                         wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxSUNKEN_BORDER);
    textCtrlBorderless->SetMinSize(wxSize(150, -1));

    wxStaticText* lblL = new wxStaticText(&dlg, wxID_ANY, GetTranslation("HkMoveLeft", "Move Monitor Left:"));
    wxStaticText* textCtrlMoveLeft = new wxStaticText(&dlg, wxID_ANY, getHotkeyString(hkMoveLeft), 
                                                       wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxSUNKEN_BORDER);
    textCtrlMoveLeft->SetMinSize(wxSize(150, -1));

    wxStaticText* lblR = new wxStaticText(&dlg, wxID_ANY, GetTranslation("HkMoveRight", "Move Monitor Right:"));
    wxStaticText* textCtrlMoveRight = new wxStaticText(&dlg, wxID_ANY, getHotkeyString(hkMoveRight), 
                                                        wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER | wxSUNKEN_BORDER);
    textCtrlMoveRight->SetMinSize(wxSize(150, -1));

    gridSizer->Add(lblB, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(textCtrlBorderless, 1, wxEXPAND);
    gridSizer->Add(lblL, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(textCtrlMoveLeft, 1, wxEXPAND);
    gridSizer->Add(lblR, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(textCtrlMoveRight, 1, wxEXPAND);

    mainSizer->Add(gridSizer, 1, wxEXPAND | wxALL, 15);

    wxStaticText* lblHint = new wxStaticText(&dlg, wxID_ANY, 
                     GetTranslation("HotkeyHint", "Click a field and press your new keys combo."), 
                     wxDefaultPosition, wxDefaultSize, wxALIGN_CENTER_HORIZONTAL);
    lblHint->SetForegroundColour(wxColor(120, 120, 120));
    mainSizer->Add(lblHint, 0, wxALIGN_CENTER | wxBOTTOM, 10);

    wxSizer* btnSizer = dlg.CreateButtonSizer(wxOK | wxCANCEL);
    mainSizer->Add(btnSizer, 0, wxALIGN_RIGHT | wxALL, 10);

    dlg.SetSizer(mainSizer);

    HotkeyConfig* currentEditing = nullptr;
    wxStaticText* currentEditingCtrl = nullptr;
    
    auto setupKeyCapture = [&](wxStaticText* ctrl, HotkeyConfig& config) {
        ctrl->Bind(wxEVT_LEFT_UP, [&currentEditing, &currentEditingCtrl, &config, ctrl](wxMouseEvent& evt) {
            if (currentEditingCtrl && currentEditing) {
                currentEditingCtrl->SetBackgroundColour(wxNullColour);
                currentEditingCtrl->Refresh();
            }
            
            currentEditing = &config;
            currentEditingCtrl = ctrl;
            ctrl->SetLabel("Press keys...");
            ctrl->SetBackgroundColour(wxColor(255, 255, 200));
            ctrl->Refresh();
            evt.Skip();
        });
    };

    setupKeyCapture(textCtrlBorderless, hkBorderless);
    setupKeyCapture(textCtrlMoveLeft, hkMoveLeft);
    setupKeyCapture(textCtrlMoveRight, hkMoveRight);
    
    dlg.Bind(wxEVT_CHAR_HOOK, [&currentEditing, &currentEditingCtrl, getHotkeyString](wxKeyEvent& event) {
        if (!currentEditing) {
            event.Skip();
            return;
        }
        
        int modifiers = 0;
        if (wxGetKeyState(WXK_CONTROL)) modifiers |= MOD_CONTROL;
        if (wxGetKeyState(WXK_ALT)) modifiers |= MOD_ALT;
        if (wxGetKeyState(WXK_SHIFT)) modifiers |= MOD_SHIFT;
        if (wxGetKeyState(WXK_WINDOWS_LEFT) || wxGetKeyState(WXK_WINDOWS_RIGHT)) modifiers |= MOD_WIN;

        int vkCode = event.GetKeyCode();
        
        if (vkCode == WXK_CONTROL || vkCode == WXK_ALT || vkCode == WXK_SHIFT || 
            vkCode == WXK_WINDOWS_LEFT || vkCode == WXK_WINDOWS_RIGHT) {
            event.Skip();
            return;
        }

        currentEditing->modifiers = modifiers;
        currentEditing->vkCode = vkCode;
        currentEditingCtrl->SetLabel(getHotkeyString(*currentEditing));
        currentEditingCtrl->SetBackgroundColour(wxNullColour);
        currentEditingCtrl->Refresh();
        
        currentEditing = nullptr;
        currentEditingCtrl = nullptr;
    });
    
    dlg.Bind(wxEVT_RIGHT_DOWN, [&currentEditing, &currentEditingCtrl, getHotkeyString](wxMouseEvent& event) {
        if (!currentEditing) {
            event.Skip();
            return;
        }
        
        int modifiers = 0;
        if (event.ControlDown()) modifiers |= MOD_CONTROL;
        if (event.AltDown()) modifiers |= MOD_ALT;
        if (event.ShiftDown()) modifiers |= MOD_SHIFT;
        if (event.MetaDown()) modifiers |= MOD_WIN;

        int vkCode = VK_RBUTTON;
        currentEditing->modifiers = modifiers;
        currentEditing->vkCode = vkCode;
        currentEditingCtrl->SetLabel(getHotkeyString(*currentEditing));
        currentEditingCtrl->SetBackgroundColour(wxNullColour);
        currentEditingCtrl->Refresh();
        
        currentEditing = nullptr;
        currentEditingCtrl = nullptr;
    });
    
    dlg.Bind(wxEVT_MIDDLE_DOWN, [&currentEditing, &currentEditingCtrl, getHotkeyString](wxMouseEvent& event) {
        if (!currentEditing) {
            event.Skip();
            return;
        }
        
        int modifiers = 0;
        if (wxGetKeyState(WXK_CONTROL)) modifiers |= MOD_CONTROL;
        if (wxGetKeyState(WXK_ALT)) modifiers |= MOD_ALT;
        if (wxGetKeyState(WXK_SHIFT)) modifiers |= MOD_SHIFT;
        if (wxGetKeyState(WXK_WINDOWS_LEFT) || wxGetKeyState(WXK_WINDOWS_RIGHT)) modifiers |= MOD_WIN;

        int vkCode = VK_MBUTTON;
        currentEditing->modifiers = modifiers;
        currentEditing->vkCode = vkCode;
        currentEditingCtrl->SetLabel(getHotkeyString(*currentEditing));
        currentEditingCtrl->SetBackgroundColour(wxNullColour);
        currentEditingCtrl->Refresh();
        
        currentEditing = nullptr;
        currentEditingCtrl = nullptr;
    });
    
    dlg.Bind(wxEVT_AUX1_DOWN, [&currentEditing, &currentEditingCtrl, getHotkeyString](wxMouseEvent& event) {
        if (!currentEditing) {
            event.Skip();
            return;
        }
        
        int modifiers = 0;
        if (wxGetKeyState(WXK_CONTROL)) modifiers |= MOD_CONTROL;
        if (wxGetKeyState(WXK_ALT)) modifiers |= MOD_ALT;
        if (wxGetKeyState(WXK_SHIFT)) modifiers |= MOD_SHIFT;
        if (wxGetKeyState(WXK_WINDOWS_LEFT) || wxGetKeyState(WXK_WINDOWS_RIGHT)) modifiers |= MOD_WIN;

        int vkCode = VK_XBUTTON1;
        currentEditing->modifiers = modifiers;
        currentEditing->vkCode = vkCode;
        currentEditingCtrl->SetLabel(getHotkeyString(*currentEditing));
        currentEditingCtrl->SetBackgroundColour(wxNullColour);
        currentEditingCtrl->Refresh();
        
        currentEditing = nullptr;
        currentEditingCtrl = nullptr;
    });
    
    dlg.Bind(wxEVT_AUX2_DOWN, [&currentEditing, &currentEditingCtrl, getHotkeyString](wxMouseEvent& event) {
        if (!currentEditing) {
            event.Skip();
            return;
        }
        
        int modifiers = 0;
        if (wxGetKeyState(WXK_CONTROL)) modifiers |= MOD_CONTROL;
        if (wxGetKeyState(WXK_ALT)) modifiers |= MOD_ALT;
        if (wxGetKeyState(WXK_SHIFT)) modifiers |= MOD_SHIFT;
        if (wxGetKeyState(WXK_WINDOWS_LEFT) || wxGetKeyState(WXK_WINDOWS_RIGHT)) modifiers |= MOD_WIN;

        int vkCode = VK_XBUTTON2;
        currentEditing->modifiers = modifiers;
        currentEditing->vkCode = vkCode;
        currentEditingCtrl->SetLabel(getHotkeyString(*currentEditing));
        currentEditingCtrl->SetBackgroundColour(wxNullColour);
        currentEditingCtrl->Refresh();
        
        currentEditing = nullptr;
        currentEditingCtrl = nullptr;
    });

    dlg.Bind(wxEVT_LEFT_DOWN, [&currentEditing, &currentEditingCtrl](wxMouseEvent& event) {
        if (currentEditing) {
            currentEditingCtrl->SetBackgroundColour(wxNullColour);
            currentEditingCtrl->Refresh();
            currentEditing = nullptr;
            currentEditingCtrl = nullptr;
        }
        event.Skip();
    });

    if (dlg.ShowModal() == wxID_OK)
    {
        hotkeyManager.SetBorderlessHotkey(hkBorderless);
        hotkeyManager.SetMoveLeftHotkey(hkMoveLeft);
        hotkeyManager.SetMoveRightHotkey(hkMoveRight);
        hotkeyManager.RegisterHotkeys((HWND)this->GetHWND());
        
        configManager.SavePersistentConfig(hotkeyManager, minimizeToTray, isDarkMode);
    }
    else
    {
        hotkeyManager.RegisterHotkeys((HWND)this->GetHWND());
    }
}
