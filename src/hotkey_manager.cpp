#include "hotkey_manager.h"

HotkeyManager::HotkeyManager()
{
    hkBorderless = { MOD_CONTROL | MOD_ALT, 'B' };
    hkMoveLeft   = { MOD_CONTROL | MOD_ALT, VK_LEFT };
    hkMoveRight  = { MOD_CONTROL | MOD_ALT, VK_RIGHT };
}

void HotkeyManager::RegisterHotkeys(HWND hwnd)
{
    if (!hwnd) return;
    
    ::RegisterHotKey(hwnd, HOTKEY_BORDERLESS_ID, hkBorderless.modifiers, hkBorderless.vkCode);
    ::RegisterHotKey(hwnd, HOTKEY_MOVE_LEFT_ID, hkMoveLeft.modifiers, hkMoveLeft.vkCode);
    ::RegisterHotKey(hwnd, HOTKEY_MOVE_RIGHT_ID, hkMoveRight.modifiers, hkMoveRight.vkCode);
}

void HotkeyManager::UnregisterHotkeys(HWND hwnd)
{
    if (!hwnd) return;
    
    ::UnregisterHotKey(hwnd, HOTKEY_BORDERLESS_ID);
    ::UnregisterHotKey(hwnd, HOTKEY_MOVE_LEFT_ID);
    ::UnregisterHotKey(hwnd, HOTKEY_MOVE_RIGHT_ID);
}