#pragma once
#include <windows.h>
#include <memory>
#include "TextEditor.hpp"
#include "SettingsDialog.hpp"

class EditorWindow {
public:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    static EditorWindow* Create(HINSTANCE hInstance, int nCmdShow);

    bool ProcessMessages();
    void Show(int nCmdShow);

private:
    EditorWindow(HWND hwnd);
    LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
    void CreateMenus();

    HWND hwnd;
    std::unique_ptr<TextEditor> editor;
    HMENU hMenu;

    static constexpr const wchar_t* CLASS_NAME = L"HoodRDEditorWindow";
};
