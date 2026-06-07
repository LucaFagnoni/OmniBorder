#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include "types.h"
#include <wx/taskbar.h>

class MainWindow;

class TrayIcon : public wxTaskBarIcon
{
public:
    TrayIcon(MainWindow *parent);
    
    virtual wxMenu *CreatePopupMenu() override;
    
    void HideTrayIcon();
    void ShowTrayIcon();
    
private:
    MainWindow *frame;
    bool isIconVisible;
    
    void OnLeftDoubleClick(wxTaskBarIconEvent &event);
    void OnShowInterface(wxCommandEvent &event);
    void OnExit(wxCommandEvent &event);
    
    wxDECLARE_EVENT_TABLE();
};

#endif