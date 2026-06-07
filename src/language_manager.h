#ifndef LANGUAGE_MANAGER_H
#define LANGUAGE_MANAGER_H

#include <wx/wx.h>
#include <wx/fileconf.h>
#include <map>
#include <string>

class LanguageManager
{
public:
    LanguageManager();
    
    void LoadTranslations(const wxString &langCode);
    wxString GetTranslation(const wxString& key, const wxString& fallback = "") const;
    
    const wxString& GetCurrentLanguage() const { return currentLanguage; }
    void SetCurrentLanguage(const wxString& lang) { currentLanguage = lang; }
    
    static wxString DetectSystemLanguage();
    
private:
    std::map<wxString, wxString> txt;
    wxString currentLanguage;
    
    wxString ReadKey(const wxFileConfig& config, const wxString &sec, const wxString &k) const;
};

#endif