#include "Settings.hpp"
#include <shlobj.h>
#include <filesystem>
#include <algorithm>

static int CALLBACK EnumFontProc(const LOGFONTW* lpelfe, const TEXTMETRICW* lpntme, DWORD FontType, LPARAM lParam) {
    if (FontType & TRUETYPE_FONTTYPE) { // Only TrueType fonts
        auto fonts = reinterpret_cast<std::vector<std::wstring>*>(lParam);
        fonts->push_back(lpelfe->lfFaceName);
    }
    return 1;
}

std::vector<std::wstring> Settings::getAvailableFonts() const {
    std::vector<std::wstring> fonts;
    HDC hdc = GetDC(NULL);
    LOGFONTW lf = { 0 };
    lf.lfCharSet = DEFAULT_CHARSET;
    
    EnumFontFamiliesExW(hdc, &lf, EnumFontProc, reinterpret_cast<LPARAM>(&fonts), 0);
    
    ReleaseDC(NULL, hdc);
    
    // Remove duplicates
    std::sort(fonts.begin(), fonts.end());
    fonts.erase(std::unique(fonts.begin(), fonts.end()), fonts.end());
    
    return fonts;
}

std::wstring Settings::getSettingsPath() const {
    wchar_t appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_APPDATA, NULL, 0, appDataPath))) {
        std::filesystem::path settingsPath = appDataPath;
        settingsPath /= L"HoodRD";
        settingsPath /= L"settings.ini";
        return settingsPath;
    }
    return L"settings.ini"; // Fallback to current directory
}

void Settings::load() {
    std::wstring settingsPath = getSettingsPath();
    wchar_t buffer[256];

    // Font settings
    GetPrivateProfileStringW(L"Font", L"Name", L"Consolas", buffer, 256, settingsPath.c_str());
    fontName = buffer;
    fontSize = GetPrivateProfileIntW(L"Font", L"Size", 14, settingsPath.c_str());
    fontBold = GetPrivateProfileIntW(L"Font", L"Bold", 0, settingsPath.c_str()) != 0;

    // Theme settings
    isDarkMode = GetPrivateProfileIntW(L"Theme", L"DarkMode", 0, settingsPath.c_str()) != 0;
    currentTheme = isDarkMode ? Theme::Dark() : Theme::Light();

    // Editor settings
    showLineNumbers = GetPrivateProfileIntW(L"Editor", L"ShowLineNumbers", 1, settingsPath.c_str()) != 0;
    wordWrap = GetPrivateProfileIntW(L"Editor", L"WordWrap", 0, settingsPath.c_str()) != 0;
    tabSize = GetPrivateProfileIntW(L"Editor", L"TabSize", 4, settingsPath.c_str());
}

void Settings::save() {
    std::wstring settingsPath = getSettingsPath();
    
    // Create directory if it doesn't exist
    std::filesystem::path dirPath = std::filesystem::path(settingsPath).parent_path();
    std::filesystem::create_directories(dirPath);

    // Font settings
    WritePrivateProfileStringW(L"Font", L"Name", fontName.c_str(), settingsPath.c_str());
    wchar_t buffer[32];
    _itow_s(fontSize, buffer, 10);
    WritePrivateProfileStringW(L"Font", L"Size", buffer, settingsPath.c_str());
    WritePrivateProfileStringW(L"Font", L"Bold", fontBold ? L"1" : L"0", settingsPath.c_str());

    // Theme settings
    WritePrivateProfileStringW(L"Theme", L"DarkMode", isDarkMode ? L"1" : L"0", settingsPath.c_str());

    // Editor settings
    WritePrivateProfileStringW(L"Editor", L"ShowLineNumbers", showLineNumbers ? L"1" : L"0", settingsPath.c_str());
    WritePrivateProfileStringW(L"Editor", L"WordWrap", wordWrap ? L"1" : L"0", settingsPath.c_str());
    _itow_s(tabSize, buffer, 10);
    WritePrivateProfileStringW(L"Editor", L"TabSize", buffer, settingsPath.c_str());
}
