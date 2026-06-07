#include "tray_icon.h"
#include "main.h"
#include <wx/artprov.h>

wxBEGIN_EVENT_TABLE(TrayIcon, wxTaskBarIcon)
    EVT_TASKBAR_LEFT_DCLICK(TrayIcon::OnLeftDoubleClick)
    EVT_MENU(ID_TRAY_SHOW_INTERFACE, TrayIcon::OnShowInterface)
    EVT_MENU(ID_MENU_EXIT, TrayIcon::OnExit)
wxEND_EVENT_TABLE()

TrayIcon::TrayIcon(MainWindow *parent) : frame(parent)
{
    isIconVisible = true;
    wxIcon icon = wxIcon("aaaa_MAIN_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
    if (icon.IsOk())
    {
        SetIcon(icon, "OmniBorder Pro");
    }
    else
    {
        SetIcon(wxArtProvider::GetIcon(wxART_EXECUTABLE_FILE, wxART_OTHER, wxSize(16, 16)), "OmniBorder Pro");
    }
}

wxMenu *TrayIcon::CreatePopupMenu()
{
    wxMenu *menu = new wxMenu;
    wxString labelShow = frame->GetTranslation("ShowInterface", "Show Interface");
    wxString labelExit = frame->GetTranslation("Exit", "Exit");
    menu->Append(ID_TRAY_SHOW_INTERFACE, labelShow);
    menu->AppendSeparator();
    menu->Append(ID_MENU_EXIT, labelExit);
    return menu;
}

void TrayIcon::OnLeftDoubleClick(wxTaskBarIconEvent &WXUNUSED(event)) 
{ 
    frame->ShowMainWindow(); 
}

void TrayIcon::OnShowInterface(wxCommandEvent &WXUNUSED(event)) 
{ 
    frame->ShowMainWindow(); 
}

void TrayIcon::OnExit(wxCommandEvent &WXUNUSED(event)) 
{ 
    frame->Destroy(); 
}

void TrayIcon::HideTrayIcon()
{
    if (isIconVisible)
    {
        wxTaskBarIcon::RemoveIcon();
        isIconVisible = false;
    }
}

void TrayIcon::ShowTrayIcon()
{
    if (!isIconVisible)
    {
        wxIcon icon = wxIcon("aaaa_MAIN_ICON", wxBITMAP_TYPE_ICO_RESOURCE);
        if (icon.IsOk())
        {
            SetIcon(icon, "OmniBorder Pro");
        }
        else
        {
            SetIcon(wxArtProvider::GetIcon(wxART_EXECUTABLE_FILE, wxART_OTHER, wxSize(16, 16)), "OmniBorder Pro");
        }
        isIconVisible = true;
    }
}