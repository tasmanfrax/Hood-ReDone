#pragma once
#include <string>
#include <windows.h>
#include <vector>
#include <fstream>

struct Theme {
    COLORREF background;
    COLORREF text;
    COLORREF cursor;
    COLORREF selection;
    COLORREF lineNumber;
    COLORREF statusBar;
    COLORREF statusText;
    
    static Theme Dark() {
        return {
            RGB(30, 30, 30),    // background
            RGB(220, 220, 220), // text
            RGB(200, 200, 200), // cursor
            RGB(60, 80, 100),   // selection
            RGB(100, 100, 100), // lineNumber
            RGB(45, 45, 45),    // statusBar
            RGB(180, 180, 180)  // statusText
        };
    }
    
    static Theme Light() {
        return {
            RGB(250, 250, 250), // background
            RGB(30, 30, 30),    // text
            RGB(0, 0, 0),       // cursor
            RGB(200, 220, 250), // selection
            RGB(120, 120, 120), // lineNumber
            RGB(240, 240, 240), // statusBar
            RGB(60, 60, 60)     // statusText
        };
    }
};

class Settings {
public:
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    void load();
    void save();
    
    // Font settings
    std::wstring fontName = L"Consolas";
    int fontSize = 14;
    bool fontBold = false;
    
    // Theme settings
    bool isDarkMode = false;
    Theme currentTheme = Theme::Light();
    
    // Editor settings
    bool showLineNumbers = true;
    bool wordWrap = false;
    int tabSize = 4;
    
    // Available fonts
    std::vector<std::wstring> getAvailableFonts() const;

private:
    Settings() { load(); }
    Settings(const Settings&) = delete;
    void operator=(const Settings&) = delete;
    
    std::wstring getSettingsPath() const;
};
