#include "logger.h"
#include <wx/dir.h>
#include <wx/app.h>

Logger& Logger::Instance()
{
    static Logger instance;
    return instance;
}

Logger::Logger()
{
}

Logger::~Logger()
{
    if (m_logFile.IsOpened())
    {
        m_logFile.Close();
    }
}

void Logger::Init()
{
    // Create logs directory if it doesn't exist
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxString exeDir = wxFileName(exePath).GetPath();
    wxString logsDir = exeDir + "/logs";
    
    if (!wxDirExists(logsDir))
    {
        wxFileName::Mkdir(logsDir, wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
    }
    
    // Initialize with current date
    m_lastDateString = GetCurrentDateString();
    m_currentLogFile = GetLogFilePath();
    
    // Open log file
    if (m_logFile.IsOpened())
    {
        m_logFile.Close();
    }
    m_logFile.Open(m_currentLogFile, wxFile::write_append);
    
    LogInfo("========================================");
    LogInfo("Application started");
    LogInfo("Log file: " + m_currentLogFile);
    LogInfo("========================================");
}

void Logger::Log(const wxString& message)
{
    wxString timestamp = wxDateTime::Now().Format("%Y-%m-%d %H:%M:%S");
    wxString logMessage = wxString::Format("[%s] %s\n", timestamp, message);
    
    // Check if date has changed
    wxString currentDate = GetCurrentDateString();
    if (currentDate != m_lastDateString)
    {
        if (m_logFile.IsOpened())
        {
            m_logFile.Close();
        }
        m_lastDateString = currentDate;
        m_currentLogFile = GetLogFilePath();
        m_logFile.Open(m_currentLogFile, wxFile::write_append);
        LogInfo("New day - new log file created");
    }
    
    // Write to file
    if (m_logFile.IsOpened())
    {
        m_logFile.Write(logMessage);
        m_logFile.Flush();
    }
    
    // Also output to console if available
    #ifdef _DEBUG
    wxLogDebug("%s", message);
    #endif
}

void Logger::LogError(const wxString& message)
{
    Log("[ERROR] " + message);
}

void Logger::LogInfo(const wxString& message)
{
    Log("[INFO] " + message);
}

void Logger::LogDebug(const wxString& message)
{
    #ifdef _DEBUG
    Log("[DEBUG] " + message);
    #endif
}

wxString Logger::GetCurrentDateString() const
{
    return wxDateTime::Now().Format("%Y-%m-%d");
}

wxString Logger::GetLogFilePath() const
{
    wxString exePath = wxStandardPaths::Get().GetExecutablePath();
    wxString exeDir = wxFileName(exePath).GetPath();
    wxString dateStr = GetCurrentDateString();
    return wxString::Format("%s/logs/%s.log", exeDir, dateStr);
}