#ifndef LOGGER_H
#define LOGGER_H

#include <wx/wx.h>
#include <wx/filename.h>
#include <wx/file.h>
#include <wx/datetime.h>
#include <wx/stdpaths.h>
#include <wx/log.h>
#include <string>

class Logger
{
public:
    static Logger& Instance();
    
    void Init();
    void Log(const wxString& message);
    void LogError(const wxString& message);
    void LogInfo(const wxString& message);
    void LogDebug(const wxString& message);
    
private:
    Logger();
    ~Logger();
    
    wxString GetLogFilePath() const;
    wxString GetCurrentDateString() const;
    
    wxString m_currentLogFile;
    wxString m_lastDateString;
    wxFile m_logFile;
    
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
};

// Macros para facilitar el logging
#define LOG_MSG(msg) Logger::Instance().Log(msg)
#define LOG_INFO(msg) Logger::Instance().LogInfo(msg)
#define LOG_ERROR(msg) Logger::Instance().LogError(msg)
#define LOG_DEBUG(msg) Logger::Instance().LogDebug(msg)

#endif