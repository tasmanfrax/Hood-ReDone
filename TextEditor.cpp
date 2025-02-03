#include "TextEditor.hpp"
#include <fstream>
#include <algorithm>
#include <cmath>
#include <string>

TextEditor::TextEditor(HWND hwnd) : hwnd(hwnd), buffer(1, "") {
    createFont();
    createBuffers();
}

TextEditor::~TextEditor() {
    destroyBuffers();
    if (hFont) DeleteObject(hFont);
}

void TextEditor::createFont() {
    Settings& settings = Settings::getInstance();
    
    LOGFONTW lf = { 0 };
    lf.lfHeight = -MulDiv(settings.fontSize, GetDeviceCaps(GetDC(hwnd), LOGPIXELSY), 72);
    lf.lfWeight = settings.fontBold ? FW_BOLD : FW_NORMAL;
    wcscpy_s(lf.lfFaceName, settings.fontName.c_str());
    
    if (hFont) DeleteObject(hFont);
    hFont = CreateFontIndirectW(&lf);
    
    // Calculate metrics
    HDC hdc = GetDC(hwnd);
    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    
    TEXTMETRICW tm;
    GetTextMetricsW(hdc, &tm);
    charWidth = tm.tmAveCharWidth;
    charHeight = tm.tmHeight + tm.tmExternalLeading;
    
    SelectObject(hdc, hOldFont);
    ReleaseDC(hwnd, hdc);
    
    // Update line number width based on total lines
    size_t maxLines = buffer.size();
    lineNumberWidth = (int)log10(maxLines + 1) + 1;
    lineNumberWidth = lineNumberWidth * charWidth + 10; // Add some padding
}

void TextEditor::createBuffers() {
    HDC hdc = GetDC(hwnd);
    memDC = CreateCompatibleDC(hdc);
    memBitmap = CreateCompatibleBitmap(hdc, clientWidth, clientHeight);
    oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
    ReleaseDC(hwnd, hdc);
}

void TextEditor::destroyBuffers() {
    if (memDC) {
        SelectObject(memDC, oldBitmap);
        DeleteDC(memDC);
        memDC = nullptr;
    }
    if (memBitmap) {
        DeleteObject(memBitmap);
        memBitmap = nullptr;
    }
}

void TextEditor::resize(int width, int height) {
    clientWidth = width;
    clientHeight = height;
    destroyBuffers();
    createBuffers();
    updateScrollInfo();
}

void TextEditor::loadFile(const std::string& fname) {
    filename = fname;
    std::ifstream file(fname);
    if (file.is_open()) {
        buffer.clear();
        std::string line;
        while (std::getline(file, line)) {
            buffer.push_back(line);
        }
        if (buffer.empty()) buffer.push_back("");
        file.close();
        isModified = false;
        updateScrollInfo();
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void TextEditor::saveFile() {
    if (filename.empty()) {
        // TODO: Show file dialog
        return;
    }
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& line : buffer) {
            file << line << "\n";
        }
        file.close();
        isModified = false;
    }
}

void TextEditor::handleChar(WPARAM wParam) {
    if (wParam >= 32 && wParam <= 126) { // Printable characters
        insertChar(static_cast<char>(wParam));
        ensureCursorVisible();
        InvalidateRect(hwnd, NULL, TRUE);
    }
}

void TextEditor::handleKeyDown(WPARAM wParam) {
    Settings& settings = Settings::getInstance();
    switch (wParam) {
        case VK_LEFT:
            if (cursorX > 0) cursorX--;
            break;
        case VK_RIGHT:
            if (cursorX < buffer[cursorY].length()) cursorX++;
            break;
        case VK_UP:
            if (cursorY > 0) {
                cursorY--;
                cursorX = std::min(cursorX, buffer[cursorY].length());
            }
            break;
        case VK_DOWN:
            if (cursorY < buffer.size() - 1) {
                cursorY++;
                cursorX = std::min(cursorX, buffer[cursorY].length());
            }
            break;
        case VK_RETURN:
            insertChar('\n');
            break;
        case VK_TAB:
            for (int i = 0; i < settings.tabSize; i++) {
                insertChar(' ');
            }
            break;
        case VK_BACK:
            if (cursorX > 0) {
                buffer[cursorY].erase(cursorX - 1, 1);
                cursorX--;
                isModified = true;
            } else if (cursorY > 0) {
                cursorX = buffer[cursorY - 1].length();
                buffer[cursorY - 1] += buffer[cursorY];
                buffer.erase(buffer.begin() + cursorY);
                cursorY--;
                isModified = true;
            }
            break;
    }
    ensureCursorVisible();
    InvalidateRect(hwnd, NULL, TRUE);
}

void TextEditor::insertChar(char ch) {
    if (ch == '\n') {
        std::string rest = buffer[cursorY].substr(cursorX);
        buffer[cursorY].erase(cursorX);
        buffer.insert(buffer.begin() + cursorY + 1, rest);
        cursorX = 0;
        cursorY++;
    } else {
        buffer[cursorY].insert(cursorX, 1, ch);
        cursorX++;
    }
    isModified = true;
}

void TextEditor::render(HDC hdc) {
    Settings& settings = Settings::getInstance();
    Theme& theme = settings.currentTheme;

    // Clear background
    RECT rect;
    GetClientRect(hwnd, &rect);
    HBRUSH hBrush = CreateSolidBrush(theme.background);
    FillRect(memDC, &rect, hBrush);
    DeleteObject(hBrush);

    // Draw line numbers if enabled
    if (settings.showLineNumbers) {
        drawLineNumbers(memDC);
    }

    // Draw text
    drawText(memDC);
    drawStatusBar(memDC);

    // Copy to screen
    BitBlt(hdc, 0, 0, clientWidth, clientHeight, memDC, 0, 0, SRCCOPY);
}

void TextEditor::drawLineNumbers(HDC hdc) {
    Settings& settings = Settings::getInstance();
    Theme& theme = settings.currentTheme;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.lineNumber);
    SelectObject(hdc, hFont);

    for (size_t i = 0; i < buffer.size(); i++) {
        POINT pos = getCharPosition(i, 0);
        std::string lineNum = std::to_string(i + 1);
        TextOutA(hdc, 5, pos.y, lineNum.c_str(), static_cast<int>(lineNum.length()));
    }

    // Draw separator line
    RECT rect = { lineNumberWidth - 2, 0, lineNumberWidth - 1, clientHeight };
    HBRUSH hBrush = CreateSolidBrush(theme.lineNumber);
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

void TextEditor::drawText(HDC hdc) {
    Settings& settings = Settings::getInstance();
    Theme& theme = settings.currentTheme;

    SelectObject(hdc, hFont);
    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.text);

    int xOffset = settings.showLineNumbers ? lineNumberWidth : 0;

    for (size_t i = 0; i < buffer.size(); i++) {
        POINT pos = getCharPosition(i, 0);
        TextOutA(hdc, pos.x + xOffset, pos.y, buffer[i].c_str(), static_cast<int>(buffer[i].length()));
    }

    // Draw cursor
    POINT cursorPos = getCharPosition(cursorY, cursorX);
    cursorPos.x += xOffset;
    RECT cursorRect = {
        cursorPos.x, cursorPos.y,
        cursorPos.x + 2, cursorPos.y + charHeight
    };
    HBRUSH hBrush = CreateSolidBrush(theme.cursor);
    FillRect(hdc, &cursorRect, hBrush);
    DeleteObject(hBrush);
}

void TextEditor::drawStatusBar(HDC hdc) {
    Settings& settings = Settings::getInstance();
    Theme& theme = settings.currentTheme;

    RECT rect;
    GetClientRect(hwnd, &rect);
    rect.top = rect.bottom - charHeight;

    HBRUSH hBrush = CreateSolidBrush(theme.statusBar);
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, theme.statusText);

    std::string status = " File: " + (filename.empty() ? "Untitled" : filename) +
                        " | Line: " + std::to_string(cursorY + 1) +
                        "/" + std::to_string(buffer.size()) +
                        " | Col: " + std::to_string(cursorX + 1) +
                        " | " + (isModified ? "Modified" : "Saved");

    TextOutA(hdc, rect.left, rect.top, status.c_str(), static_cast<int>(status.length()));
}

POINT TextEditor::getCharPosition(size_t line, size_t col) const {
    Settings& settings = Settings::getInstance();
    return {
        static_cast<LONG>((col * charWidth) - scrollX),
        static_cast<LONG>((line * charHeight) - scrollY)
    };
}

void TextEditor::updateScrollInfo() {
    Settings& settings = Settings::getInstance();
    
    // Calculate maximum scroll values
    size_t maxLineLength = 0;
    for (const auto& line : buffer) {
        maxLineLength = std::max(maxLineLength, line.length());
    }

    int xOffset = settings.showLineNumbers ? lineNumberWidth : 0;
    maxScrollX = static_cast<int>(maxLineLength * charWidth - clientWidth + xOffset + 20);
    maxScrollY = static_cast<int>(buffer.size() * charHeight - clientHeight + charHeight + 20);

    maxScrollX = std::max(0, maxScrollX);
    maxScrollY = std::max(0, maxScrollY);

    // Update scroll bars
    SCROLLINFO si = { sizeof(SCROLLINFO) };
    si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
    si.nMin = 0;

    si.nMax = maxScrollX;
    si.nPage = clientWidth;
    si.nPos = scrollX;
    SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);

    si.nMax = maxScrollY;
    si.nPage = clientHeight;
    si.nPos = scrollY;
    SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
}

void TextEditor::ensureCursorVisible() {
    Settings& settings = Settings::getInstance();
    POINT cursorPos = getCharPosition(cursorY, cursorX);
    int xOffset = settings.showLineNumbers ? lineNumberWidth : 0;
    
    // Horizontal scrolling
    if (cursorPos.x < xOffset) {
        scrollX += (cursorPos.x - xOffset);
    } else if (cursorPos.x >= clientWidth - charWidth) {
        scrollX += (cursorPos.x - clientWidth + charWidth);
    }

    // Vertical scrolling
    if (cursorPos.y < 0) {
        scrollY += cursorPos.y;
    } else if (cursorPos.y >= clientHeight - charHeight * 2) {
        scrollY += (cursorPos.y - clientHeight + charHeight * 2);
    }

    scrollX = std::max(0, std::min(scrollX, maxScrollX));
    scrollY = std::max(0, std::min(scrollY, maxScrollY));

    updateScrollInfo();
}

void TextEditor::applySettings() {
    createFont();
    updateScrollInfo();
    InvalidateRect(hwnd, NULL, TRUE);
}

bool TextEditor::queryClose() {
    if (!isModified) return true;
    return MessageBoxW(hwnd, L"Do you want to save changes?", L"Save Changes", 
                      MB_YESNOCANCEL | MB_ICONQUESTION) != IDCANCEL;
}
