#include "EditorWindow.hpp"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    EditorWindow* window = EditorWindow::Create(hInstance, nCmdShow);
    if (!window) {
        return 0;
    }

    // Main message loop
    while (window->ProcessMessages()) {
        // Additional processing can be done here
    }

    delete window;
    return 0;
}
