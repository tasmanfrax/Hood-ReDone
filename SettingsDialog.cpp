#include "SettingsDialog.hpp"
#include <commctrl.h>
#include <windowsx.h>
#include "resource.h"

#pragma comment(lib, "comctl32.lib")

INT_PTR SettingsDialog::Show(HWND parent) {
    return DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SETTINGS), parent, DialogProc);
}

INT_PTR CALLBACK SettingsDialog::DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_INITDIALOG:
            InitDialog(hwnd);
            return TRUE;

        case WM_COMMAND:
            OnCommand(hwnd, LOWORD(wParam), (HWND)lParam, HIWORD(wParam));
            return TRUE;

        case WM_CLOSE:
            EndDialog(hwnd, IDCANCEL);
            return TRUE;
    }
    return FALSE;
}

void SettingsDialog::InitDialog(HWND hwnd) {
    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    Settings& settings = Settings::getInstance();

    // Initialize font combo box
    HWND hFontCombo = GetDlgItem(hwnd, IDC_FONT_COMBO);
    auto fonts = settings.getAvailableFonts();
    for (const auto& font : fonts) {
        ComboBox_AddString(hFontCombo, font.c_str());
        if (font == settings.fontName) {
            ComboBox_SetCurSel(hFontCombo, ComboBox_GetCount(hFontCombo) - 1);
        }
    }

    // Initialize font size combo box
    HWND hSizeCombo = GetDlgItem(hwnd, IDC_SIZE_COMBO);
    int sizes[] = {8, 9, 10, 11, 12, 14, 16, 18, 20, 22, 24, 26, 28, 36, 48, 72};
    for (int size : sizes) {
        wchar_t buf[8];
        _itow_s(size, buf, 10);
        ComboBox_AddString(hSizeCombo, buf);
        if (size == settings.fontSize) {
            ComboBox_SetCurSel(hSizeCombo, ComboBox_GetCount(hSizeCombo) - 1);
        }
    }

    // Initialize checkboxes
    CheckDlgButton(hwnd, IDC_BOLD_CHECK, settings.fontBold ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_DARK_MODE_CHECK, settings.isDarkMode ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_LINE_NUMBERS_CHECK, settings.showLineNumbers ? BST_CHECKED : BST_UNCHECKED);
    CheckDlgButton(hwnd, IDC_WORD_WRAP_CHECK, settings.wordWrap ? BST_CHECKED : BST_UNCHECKED);

    // Initialize tab size
    HWND hTabSize = GetDlgItem(hwnd, IDC_TAB_SIZE);
    SetDlgItemInt(hwnd, IDC_TAB_SIZE, settings.tabSize, FALSE);

    UpdatePreview(hwnd);
}

void SettingsDialog::OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
    switch (id) {
        case IDOK:
            ApplySettings(hwnd);
            EndDialog(hwnd, IDOK);
            break;

        case IDCANCEL:
            EndDialog(hwnd, IDCANCEL);
            break;

        case IDC_FONT_COMBO:
        case IDC_SIZE_COMBO:
        case IDC_BOLD_CHECK:
        case IDC_DARK_MODE_CHECK:
            if (codeNotify == CBN_SELCHANGE || codeNotify == BN_CLICKED) {
                UpdatePreview(hwnd);
            }
            break;
    }
}

void SettingsDialog::ApplySettings(HWND hwnd) {
    Settings& settings = Settings::getInstance();

    // Get font name
    HWND hFontCombo = GetDlgItem(hwnd, IDC_FONT_COMBO);
    int fontIndex = ComboBox_GetCurSel(hFontCombo);
    if (fontIndex != CB_ERR) {
        wchar_t fontName[LF_FACESIZE];
        ComboBox_GetLBText(hFontCombo, fontIndex, fontName);
        settings.fontName = fontName;
    }

    // Get font size
    HWND hSizeCombo = GetDlgItem(hwnd, IDC_SIZE_COMBO);
    int sizeIndex = ComboBox_GetCurSel(hSizeCombo);
    if (sizeIndex != CB_ERR) {
        wchar_t sizeStr[8];
        ComboBox_GetLBText(hSizeCombo, sizeIndex, sizeStr);
        settings.fontSize = _wtoi(sizeStr);
    }

    // Get other settings
    settings.fontBold = IsDlgButtonChecked(hwnd, IDC_BOLD_CHECK) == BST_CHECKED;
    settings.isDarkMode = IsDlgButtonChecked(hwnd, IDC_DARK_MODE_CHECK) == BST_CHECKED;
    settings.showLineNumbers = IsDlgButtonChecked(hwnd, IDC_LINE_NUMBERS_CHECK) == BST_CHECKED;
    settings.wordWrap = IsDlgButtonChecked(hwnd, IDC_WORD_WRAP_CHECK) == BST_CHECKED;
    settings.tabSize = GetDlgItemInt(hwnd, IDC_TAB_SIZE, NULL, FALSE);

    settings.currentTheme = settings.isDarkMode ? Theme::Dark() : Theme::Light();
    settings.save();
}

HFONT SettingsDialog::CreateGUIFont(const wchar_t* name, int size, bool bold) {
    return CreateFontW(
        -size,                    // Height (negative for character height)
        0,                        // Width
        0,                        // Escapement
        0,                        // Orientation
        bold ? FW_BOLD : FW_NORMAL, // Weight
        FALSE,                    // Italic
        FALSE,                    // Underline
        FALSE,                    // StrikeOut
        DEFAULT_CHARSET,          // CharSet
        OUT_DEFAULT_PRECIS,       // OutPrecision
        CLIP_DEFAULT_PRECIS,      // ClipPrecision
        CLEARTYPE_QUALITY,        // Quality
        DEFAULT_PITCH | FF_DONTCARE, // PitchAndFamily
        name                      // FaceName
    );
}

void SettingsDialog::UpdatePreview(HWND hwnd) {
    HWND hPreview = GetDlgItem(hwnd, IDC_PREVIEW);
    if (!hPreview) return;

    // Get current settings from dialog
    wchar_t fontName[LF_FACESIZE];
    HWND hFontCombo = GetDlgItem(hwnd, IDC_FONT_COMBO);
    int fontIndex = ComboBox_GetCurSel(hFontCombo);
    if (fontIndex != CB_ERR) {
        ComboBox_GetLBText(hFontCombo, fontIndex, fontName);
    }

    wchar_t sizeStr[8];
    HWND hSizeCombo = GetDlgItem(hwnd, IDC_SIZE_COMBO);
    int sizeIndex = ComboBox_GetCurSel(hSizeCombo);
    if (sizeIndex != CB_ERR) {
        ComboBox_GetLBText(hSizeCombo, sizeIndex, sizeStr);
    }

    bool bold = IsDlgButtonChecked(hwnd, IDC_BOLD_CHECK) == BST_CHECKED;
    bool darkMode = IsDlgButtonChecked(hwnd, IDC_DARK_MODE_CHECK) == BST_CHECKED;

    // Create and set font
    HFONT hFont = CreateGUIFont(fontName, _wtoi(sizeStr), bold);
    SendMessage(hPreview, WM_SETFONT, (WPARAM)hFont, TRUE);

    // Set colors
    Theme theme = darkMode ? Theme::Dark() : Theme::Light();
    SetClassLongPtr(hPreview, GCLP_HBRBACKGROUND, (LONG_PTR)CreateSolidBrush(theme.background));
    InvalidateRect(hPreview, NULL, TRUE);
}
