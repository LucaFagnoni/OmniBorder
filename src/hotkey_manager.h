#ifndef HOTKEY_MANAGER_H
#define HOTKEY_MANAGER_H

#include "types.h"
#include <wx/wx.h>

class HotkeyManager
{
public:
    HotkeyManager();
    
    void RegisterHotkeys(HWND hwnd);
    void UnregisterHotkeys(HWND hwnd);
    
    void SetBorderlessHotkey(const HotkeyConfig& config) { hkBorderless = config; }
    void SetMoveLeftHotkey(const HotkeyConfig& config) { hkMoveLeft = config; }
    void SetMoveRightHotkey(const HotkeyConfig& config) { hkMoveRight = config; }
    
    const HotkeyConfig& GetBorderlessHotkey() const { return hkBorderless; }
    const HotkeyConfig& GetMoveLeftHotkey() const { return hkMoveLeft; }
    const HotkeyConfig& GetMoveRightHotkey() const { return hkMoveRight; }
    
    bool IsBorderlessHotkey(int id) const { return id == HOTKEY_BORDERLESS_ID; }
    bool IsMoveLeftHotkey(int id) const { return id == HOTKEY_MOVE_LEFT_ID; }
    bool IsMoveRightHotkey(int id) const { return id == HOTKEY_MOVE_RIGHT_ID; }
    
private:
    HotkeyConfig hkBorderless;
    HotkeyConfig hkMoveLeft;
    HotkeyConfig hkMoveRight;
};

#endif