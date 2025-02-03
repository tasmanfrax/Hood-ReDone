#include "EditorWindow.hpp"
#include "resource.h"
#include <commctrl.h>

EditorWindow::EditorWindow(HWND hwnd) : hwnd(hwnd) {
    editor = std::make_unique<TextEditor>(hwnd);
    CreateMenus();
}

void EditorWindow::CreateMenus() {
    hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hEditMenu = CreatePopupMenu();
    HMENU hViewMenu = CreatePopupMenu();

    // File menu
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_NEW, L"&New");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVE_AS, L"Save &As...");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit");

    // Edit menu
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_UNDO, L"&Undo");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_REDO, L"&Redo");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_CUT, L"Cu&t");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_COPY, L"&Copy");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_PASTE, L"&Paste");

    // View menu
    AppendMenuW(hViewMenu, MF_STRING, IDM_VIEW_SETTINGS, L"&Settings...");

    // Add menus to menu bar
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");

    SetMenu(hwnd, hMenu);
}

EditorWindow* EditorWindow::Create(HINSTANCE hInstance, int nCmdShow) {
    // Register window class
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_IBEAM);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

    RegisterClassExW(&wc);

    // Create window
    HWND hwnd = CreateWindowExW(
        0,                          // Optional window styles
        CLASS_NAME,                 // Window class
        L"Hood RD Editor",         // Window text
        WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, // Window style

        // Size and position
        CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,

        nullptr,     // Parent window    
        nullptr,     // Menu
        hInstance,   // Instance handle
        nullptr      // Additional application data
    );

    if (hwnd == nullptr) {
        return nullptr;
    }

    // Create and store the window instance
    EditorWindow* window = new EditorWindow(hwnd);
    SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));

    window->Show(nCmdShow);
    return window;
}

LRESULT CALLBACK EditorWindow::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    EditorWindow* window = reinterpret_cast<EditorWindow*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
    
    if (window) {
        return window->HandleMessage(uMsg, wParam, lParam);
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

LRESULT EditorWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            editor->render(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_SIZE: {
            int width = LOWORD(lParam);
            int height = HIWORD(lParam);
            editor->resize(width, height);
            return 0;
        }

        case WM_CHAR:
            editor->handleChar(wParam);
            return 0;

        case WM_KEYDOWN:
            editor->handleKeyDown(wParam);
            return 0;

        case WM_CLOSE:
            if (editor->queryClose()) {
                DestroyWindow(hwnd);
            }
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case IDM_VIEW_SETTINGS:
                    if (SettingsDialog::Show(hwnd) == IDOK) {
                        editor->applySettings();
                    }
                    return 0;
            }
            break;

        case WM_VSCROLL:
        case WM_HSCROLL:
            // TODO: Handle scrolling
            return 0;
    }
    
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void EditorWindow::Show(int nCmdShow) {
    ShowWindow(hwnd, nCmdShow);
}

bool EditorWindow::ProcessMessages() {
    MSG msg = {};
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) {
            return false;
        }
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return true;
}
