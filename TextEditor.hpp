#pragma once
#include <vector>
#include <string>
#include <windows.h>
#include <memory>
#include "Settings.hpp"

class TextEditor {
public:
    TextEditor(HWND hwnd);
    ~TextEditor();

    void loadFile(const std::string& fname);
    void saveFile();
    void handleChar(WPARAM wParam);
    void handleKeyDown(WPARAM wParam);
    void render(HDC hdc);
    void resize(int width, int height);
    bool queryClose();
    void applySettings();

private:
    void createBuffers();
    void destroyBuffers();
    void updateScrollInfo();
    void ensureCursorVisible();
    void insertChar(char ch);
    void drawText(HDC hdc);
    void drawLineNumbers(HDC hdc);
    void drawStatusBar(HDC hdc);
    POINT getCharPosition(size_t line, size_t col) const;
    void createFont();

    HWND hwnd;
    std::vector<std::string> buffer;
    size_t cursorX = 0;
    size_t cursorY = 0;
    std::string filename;
    bool isModified = false;

    // Triple buffering
    HDC memDC = nullptr;
    HBITMAP memBitmap = nullptr;
    HBITMAP oldBitmap = nullptr;
    
    // Font and metrics
    HFONT hFont = nullptr;
    int charWidth = 0;
    int charHeight = 0;
    int clientWidth = 0;
    int clientHeight = 0;

    // Scroll info
    int scrollX = 0;
    int scrollY = 0;
    int maxScrollX = 0;
    int maxScrollY = 0;

    // Line numbers
    int lineNumberWidth = 0;
};
