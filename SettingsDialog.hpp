#pragma once
#include <windows.h>
#include "Settings.hpp"

class SettingsDialog {
public:
    static INT_PTR Show(HWND parent);

private:
    static INT_PTR CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void InitDialog(HWND hwnd);
    static void OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify);
    static void ApplySettings(HWND hwnd);
    
    static HFONT CreateGUIFont(const wchar_t* name, int size, bool bold = false);
    static void UpdatePreview(HWND hwnd);
};
