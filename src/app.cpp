#include <wx/wx.h>
#include "main.h"
#include "logger.h"

class OmniBorderApp : public wxApp
{
public:
    virtual bool OnInit() override;
    virtual int OnExit() override;
};

bool OmniBorderApp::OnInit()
{
    // Initialize logging system FIRST
    Logger::Instance().Init();
    LOG_INFO("OmniBorder application starting...");
    
    if (!wxApp::OnInit())
    {
        LOG_ERROR("wxApp::OnInit() failed");
        return false;
    }
    
    LOG_INFO("Creating MainWindow...");
    
    try
    {
        MainWindow *mainWindow = new MainWindow();
        LOG_INFO("MainWindow created successfully");
        
        LOG_INFO("Showing MainWindow...");
        mainWindow->Show(true);
        LOG_INFO("MainWindow shown successfully");
        
        return true;
    }
    catch (const std::exception& e)
    {
        LOG_ERROR(wxString::Format("Exception during initialization: %s", e.what()));
        return false;
    }
    catch (...)
    {
        LOG_ERROR("Unknown exception during initialization");
        return false;
    }
}

int OmniBorderApp::OnExit()
{
    LOG_INFO("Application exiting...");
    return wxApp::OnExit();
}

wxIMPLEMENT_APP(OmniBorderApp);