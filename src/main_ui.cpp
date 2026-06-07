#include "main.h"
#include <wx/sizer.h>
#include <wx/button.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>
#include <wx/listbox.h>
#include <wx/choice.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/font.h>

void MainWindow::CreateUI()
{
    wxFont modernFont(10, wxFONTFAMILY_DEFAULT, wxFONTSTYLE_NORMAL, wxFONTWEIGHT_NORMAL, false, "Segoe UI");
    this->SetFont(modernFont);

    menuBar = new wxMenuBar();
    menuFile = new wxMenu();
    menuFile->Append(ID_MENU_EXIT, GetTranslation("Exit", "Exit"));
    
    menuOptions = new wxMenu();
    menuLanguageSub = new wxMenu();
    menuLanguageSub->AppendRadioItem(ID_MENU_LANG_ES, GetTranslation("LangEs", "Spanish"));
    menuLanguageSub->AppendRadioItem(ID_MENU_LANG_EN, GetTranslation("LangEn", "English"));
    
    menuItemLanguageSub = menuOptions->AppendSubMenu(menuLanguageSub, GetTranslation("Language", "Language"));
    menuItemStartup = menuOptions->AppendCheckItem(ID_MENU_STARTUP, GetTranslation("StartWithWindows", "Start with Windows"));
    menuOptions->Check(ID_MENU_STARTUP, CheckStartupOnWindows());
    
    menuItemMinimizeToTray = menuOptions->AppendCheckItem(ID_MENU_MINIMIZE_TO_TRAY, "Minimize to Tray");
    menuOptions->Check(ID_MENU_MINIMIZE_TO_TRAY, minimizeToTray);
    
    menuOptions->AppendSeparator();
    menuItemHotkeys = menuOptions->Append(ID_MENU_HOTKEYS, GetTranslation("ModifyHotkeys", "Configure Hotkeys"));
    
    menuItemDarkMode = menuOptions->AppendCheckItem(ID_MENU_DARK_MODE, GetTranslation("DarkMode", "Dark Mode"));
    menuOptions->Check(ID_MENU_DARK_MODE, isDarkMode);

    menuLanguageSub->Check(languageManager.GetCurrentLanguage() == "es" ? ID_MENU_LANG_ES : ID_MENU_LANG_EN, true);
    menuHelp = new wxMenu();
    menuHelp->Append(ID_MENU_ABOUT, GetTranslation("About", "About"));
    
    menuBar->Append(menuFile, GetTranslation("File", "File"));
    menuBar->Append(menuOptions, GetTranslation("Options", "Options"));
    menuBar->Append(menuHelp, GetTranslation("Help", "Help"));
    this->SetMenuBar(menuBar);

    mainPanel = new wxPanel(this, wxID_ANY);
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer *topBarSizer = new wxBoxSizer(wxHORIZONTAL);
    textCtrlSearch = new wxTextCtrl(mainPanel, ID_SEARCH_BOX, "", wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    topBarSizer->Add(textCtrlSearch, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(topBarSizer, 0, wxEXPAND | wxALL, 5);

    wxFlexGridSizer *panelsContainerSizer = new wxFlexGridSizer(1, 3, 0, 0);
    panelsContainerSizer->AddGrowableRow(0);
    panelsContainerSizer->AddGrowableCol(0, 1);
    panelsContainerSizer->AddGrowableCol(2, 1);

    wxBoxSizer *leftPanelSizer = new wxBoxSizer(wxVERTICAL);
    labelNormalList = new wxStaticText(mainPanel, wxID_ANY, "");
    labelNormalList->SetFont(modernFont.Bold());
    listBoxNormal = new wxListBox(mainPanel, ID_NORMAL_LIST, wxDefaultPosition, wxSize(100, 100), 0, nullptr, wxLB_SINGLE | wxLB_HSCROLL);
    leftPanelSizer->Add(labelNormalList, 0, wxBOTTOM, 5);
    leftPanelSizer->Add(listBoxNormal, 1, wxEXPAND);

    wxBoxSizer *centerActionSizer = new wxBoxSizer(wxVERTICAL);
    buttonToBorderless = new wxButton(mainPanel, ID_MOVE_TO_BORDERLESS_BTN, ">>", wxDefaultPosition, wxSize(60, 40));
    buttonToNormal = new wxButton(mainPanel, ID_RESTORE_TO_NORMAL_BTN, "<<", wxDefaultPosition, wxSize(60, 40));
    centerActionSizer->Add(buttonToBorderless, 0, wxALL | wxALIGN_CENTER, 5);
    centerActionSizer->Add(buttonToNormal, 0, wxALL | wxALIGN_CENTER, 5);

    wxBoxSizer *rightPanelSizer = new wxBoxSizer(wxVERTICAL);
    labelBorderlessList = new wxStaticText(mainPanel, wxID_ANY, "");
    labelBorderlessList->SetFont(modernFont.Bold());
    listBoxBorderless = new wxListBox(mainPanel, ID_BORDERLESS_LIST, wxDefaultPosition, wxSize(100, 100), 0, nullptr, wxLB_SINGLE | wxLB_HSCROLL);
    rightPanelSizer->Add(labelBorderlessList, 0, wxBOTTOM, 5);
    rightPanelSizer->Add(listBoxBorderless, 1, wxEXPAND);

    panelsContainerSizer->Add(leftPanelSizer, 1, wxEXPAND | wxALL, 10);
    panelsContainerSizer->Add(centerActionSizer, 0, wxALIGN_CENTER | wxLEFT | wxRIGHT, 5);
    panelsContainerSizer->Add(rightPanelSizer, 1, wxEXPAND | wxALL, 10);
    mainSizer->Add(panelsContainerSizer, 1, wxEXPAND);

    wxBoxSizer *footerSizer = new wxBoxSizer(wxHORIZONTAL);
    wxBoxSizer *technicalCard = new wxBoxSizer(wxVERTICAL);
    labelDetailsBox = new wxStaticText(mainPanel, wxID_ANY, "");
    labelDetailsBox->SetFont(modernFont.Bold());
    technicalCard->Add(labelDetailsBox, 0, wxBOTTOM, 5);

    wxFlexGridSizer *gridSizer = new wxFlexGridSizer(3, 2, 5, 10);
    labelHwnd = new wxStaticText(mainPanel, wxID_ANY, "");
    statusTextHwnd = new wxStaticText(mainPanel, wxID_ANY, "");
    statusTextHwnd->SetFont(modernFont.Bold());
    
    labelSize = new wxStaticText(mainPanel, wxID_ANY, "");
    statusTextSize = new wxStaticText(mainPanel, wxID_ANY, "");
    statusTextSize->SetFont(modernFont.Bold());
    
    labelTargetMonitor = new wxStaticText(mainPanel, wxID_ANY, "");
    
    wxArrayString monitorChoices;
    monitorChoices.Add(GetTranslation("CurrentMonitor", "Current Monitor"));
    for (size_t i = 0; i < monitorManager.GetMonitors().size(); ++i)
    {
        monitorChoices.Add(wxString::Format("%s (%dx%d)", monitorManager.GetMonitors()[i].friendlyName, 
            monitorManager.GetMonitors()[i].rect.right - monitorManager.GetMonitors()[i].rect.left, 
            monitorManager.GetMonitors()[i].rect.bottom - monitorManager.GetMonitors()[i].rect.top));
    }
    choiceMonitors = new wxChoice(mainPanel, ID_MONITOR_CHOICE, wxDefaultPosition, wxDefaultSize, monitorChoices);
    choiceMonitors->SetSelection(0);

    gridSizer->Add(labelHwnd, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(statusTextHwnd, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(labelSize, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(statusTextSize, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(labelTargetMonitor, 0, wxALIGN_LEFT | wxALIGN_CENTER_VERTICAL);
    gridSizer->Add(choiceMonitors, 1, wxEXPAND | wxALIGN_CENTER_VERTICAL);
    technicalCard->Add(gridSizer, 1, wxEXPAND);

    wxBoxSizer *globalButtonsSizer = new wxBoxSizer(wxHORIZONTAL);
    buttonRestoreAll = new wxButton(mainPanel, ID_RESTORE_ALL_BTN, "", wxDefaultPosition, wxSize(150, 45));
    buttonRefresh = new wxButton(mainPanel, ID_REFRESH_BTN, "", wxDefaultPosition, wxSize(160, 45));
    globalButtonsSizer->Add(buttonRestoreAll, 0, wxRIGHT, 10);
    globalButtonsSizer->Add(buttonRefresh, 0);

    footerSizer->Add(technicalCard, 1, wxEXPAND | wxALL, 10);
    footerSizer->Add(globalButtonsSizer, 0, wxALIGN_CENTER | wxALL, 10);
    mainSizer->Add(footerSizer, 0, wxEXPAND | wxTOP, 5);

    mainPanel->SetSizer(mainSizer);
    
    windowManager.EnumerateWindows();
    FilterLists();
    ApplyTheme();
}

void MainWindow::UpdateUITexts()
{
    textCtrlSearch->SetHint(GetTranslation("SearchHint", "Search app..."));
    labelNormalList->SetLabel(GetTranslation("ActiveApps", "Active Windows"));
    labelBorderlessList->SetLabel(GetTranslation("BorderlessActive", "Borderless Apps"));
    labelDetailsBox->SetLabel(GetTranslation("TechnicalDetails", "Technical Details"));
    labelHwnd->SetLabel(GetTranslation("HwndLabel", "Window HWND:"));
    labelSize->SetLabel(GetTranslation("ResolutionLabel", "Resolution:"));
    labelTargetMonitor->SetLabel(GetTranslation("TargetMonitor", "Target Monitor:"));
    buttonRestoreAll->SetLabel(GetTranslation("RestoreAll", "Restore All"));
    buttonRefresh->SetLabel(GetTranslation("ScanSystem", "Scan System"));

    if (statusTextHwnd->GetLabel() == "Ninguno" || statusTextHwnd->GetLabel() == "None" || statusTextHwnd->GetLabel().IsEmpty())
    {
        statusTextHwnd->SetLabel(GetTranslation("None", "None"));
    }

    choiceMonitors->SetString(0, GetTranslation("CurrentMonitor", "Current Monitor"));
    menuBar->SetMenuLabel(0, GetTranslation("File", "File"));
    menuBar->SetMenuLabel(1, GetTranslation("Options", "Options"));
    menuBar->SetMenuLabel(2, GetTranslation("Help", "Help"));
    menuFile->SetLabel(ID_MENU_EXIT, GetTranslation("Exit", "Exit"));
    menuLanguageSub->SetLabel(ID_MENU_LANG_ES, GetTranslation("LangEs", "Spanish"));
    menuLanguageSub->SetLabel(ID_MENU_LANG_EN, GetTranslation("LangEn", "English"));
    menuHelp->SetLabel(ID_MENU_ABOUT, GetTranslation("About", "About"));
    
    if (menuItemStartup) menuItemStartup->SetItemLabel(GetTranslation("StartWithWindows", "Start with Windows"));
    if (menuItemLanguageSub) menuItemLanguageSub->SetItemLabel(GetTranslation("Language", "Language"));
    if (menuItemHotkeys) menuItemHotkeys->SetItemLabel(GetTranslation("ModifyHotkeys", "Configure Hotkeys"));
    
    if (menuItemMinimizeToTray)
    {
        menuItemMinimizeToTray->SetItemLabel(GetTranslation("MinimizeToTray", "Minimize to Tray"));
        menuItemMinimizeToTray->Check(minimizeToTray);
    }
    
    if (menuItemDarkMode)
    {
        menuItemDarkMode->SetItemLabel(GetTranslation("DarkMode", "Dark Mode"));
        menuItemDarkMode->Check(isDarkMode);
    }

    trayIcon->SetIcon(wxIcon("aaaa_MAIN_ICON", wxBITMAP_TYPE_ICO_RESOURCE), 
                      GetTranslation("TrayTooltip", "OmniBorder"));
    mainPanel->Layout();
}

void MainWindow::ApplyTheme()
{
    // Define colors based on dark mode state
    wxColor bgColor, inputBgColor, textTitleColor, textBodyColor, textMutedColor, btnBgColor, menuBgColor, borderColor;
    
    if (isDarkMode)
    {
        // Dark mode colors
        bgColor = wxColor(32, 32, 32);           // Very dark gray background
        inputBgColor = wxColor(45, 45, 45);       // Slightly lighter for inputs
        menuBgColor = wxColor(45, 45, 45);        // Menu background
        textTitleColor = wxColor(0, 162, 232);    // Blue accent for titles
        textBodyColor = wxColor(230, 230, 230);   // Light gray text
        textMutedColor = wxColor(160, 160, 160);  // Muted text
        btnBgColor = wxColor(55, 55, 55);         // Button background
        borderColor = wxColor(60, 60, 60);        // Dark borders
    }
    else
    {
        // Light mode colors
        bgColor = wxColor(240, 242, 245);
        inputBgColor = wxColor(255, 255, 255);
        menuBgColor = wxColor(240, 240, 240);
        textTitleColor = wxColor(12, 85, 160);
        textBodyColor = wxColor(40, 40, 40);
        textMutedColor = wxColor(110, 110, 110);
        btnBgColor = wxColor(225, 225, 225);
        borderColor = wxColor(200, 200, 200);
    }

    // Apply to main window and panel
    this->SetBackgroundColour(bgColor);
    mainPanel->SetBackgroundColour(bgColor);
    
    // Apply to labels
    labelNormalList->SetForegroundColour(textTitleColor);
    labelBorderlessList->SetForegroundColour(textTitleColor);
    labelDetailsBox->SetForegroundColour(textTitleColor);
    labelHwnd->SetForegroundColour(textMutedColor);
    labelSize->SetForegroundColour(textMutedColor);
    labelTargetMonitor->SetForegroundColour(textMutedColor);
    statusTextHwnd->SetForegroundColour(textBodyColor);
    statusTextSize->SetForegroundColour(textBodyColor);
    
    // Apply to input controls with borders
    textCtrlSearch->SetBackgroundColour(inputBgColor);
    textCtrlSearch->SetForegroundColour(textBodyColor);
    
    listBoxNormal->SetBackgroundColour(inputBgColor);
    listBoxNormal->SetForegroundColour(textBodyColor);
    
    listBoxBorderless->SetBackgroundColour(inputBgColor);
    listBoxBorderless->SetForegroundColour(textBodyColor);
    
    choiceMonitors->SetBackgroundColour(inputBgColor);
    choiceMonitors->SetForegroundColour(textBodyColor);

    // Apply to buttons
    buttonRefresh->SetBackgroundColour(btnBgColor);
    buttonRefresh->SetForegroundColour(textBodyColor);
    buttonToBorderless->SetBackgroundColour(btnBgColor);
    buttonToBorderless->SetForegroundColour(textBodyColor);
    buttonToNormal->SetBackgroundColour(btnBgColor);
    buttonToNormal->SetForegroundColour(textBodyColor);
    buttonRestoreAll->SetBackgroundColour(btnBgColor);
    buttonRestoreAll->SetForegroundColour(textBodyColor);
    
    // Set menu bar colors (limited support in wxWidgets on Windows)
    // Note: wxWidgets doesn't fully support dark mode for menus on Windows
    if (menuBar)
    {
        menuBar->SetBackgroundColour(menuBgColor);
        this->Refresh();
    }
    
    // Refresh all controls
    mainPanel->Refresh();
    this->Refresh();
    
    // Force update
    this->Update();
}