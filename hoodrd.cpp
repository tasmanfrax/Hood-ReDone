#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <conio.h>
#include <windows.h>
#include <chrono>
#include <thread>

class TextEditor {
private:
    std::vector<std::string> buffer;
    size_t cursorX = 0;
    size_t cursorY = 0;
    std::string filename;
    bool isModified = false;
    HANDLE consoleHandle;
    bool needsRedraw = true;
    
    void moveCursor(int x, int y) {
        COORD pos = {static_cast<SHORT>(x), static_cast<SHORT>(y)};
        SetConsoleCursorPosition(consoleHandle, pos);
    }

    void clearScreen() {
        system("cls");
    }

    void displayStatusBar() {
        CONSOLE_SCREEN_BUFFER_INFO csbi;
        GetConsoleScreenBufferInfo(consoleHandle, &csbi);
        int rows = csbi.srWindow.Bottom - csbi.srWindow.Top;
        
        moveCursor(0, rows - 1);
        std::cout << "\033[7m"; // Inverse colors
        std::cout << " File: " << (filename.empty() ? "Untitled" : filename);
        std::cout << " | Line: " << cursorY + 1 << "/" << buffer.size();
        std::cout << " | Col: " << cursorX + 1;
        std::cout << " | " << (isModified ? "Modified" : "Saved");
        std::cout << " | Ctrl+S: Save | Ctrl+Q: Quit";
        std::cout << "\033[0m"; // Reset colors
    }

    void refreshScreen() {
        if (!needsRedraw) return;
        clearScreen();
        // Display text
        for (size_t i = 0; i < buffer.size(); i++) {
            std::cout << buffer[i] << "\n";
        }
        displayStatusBar();
        moveCursor(cursorX, cursorY);
        needsRedraw = false;
    }

public:
    TextEditor() : buffer(1, "") {
        consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
        // Enable ANSI escape sequences
        DWORD mode;
        GetConsoleMode(consoleHandle, &mode);
        SetConsoleMode(consoleHandle, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }

    void loadFile(const std::string& fname) {
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
            needsRedraw = true;
        }
    }

    void saveFile() {
        if (filename.empty()) {
            std::cout << "Enter filename to save: ";
            std::cin >> filename;
        }
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto& line : buffer) {
                file << line << "\n";
            }
            file.close();
            isModified = false;
            needsRedraw = true;
        }
    }

    void insertChar(char ch) {
        if (ch == 13) { // Enter key
            std::string rest = buffer[cursorY].substr(cursorX);
            buffer[cursorY].erase(cursorX);
            buffer.insert(buffer.begin() + cursorY + 1, rest);
            cursorX = 0;
            cursorY++;
        } else if (ch == 8) { // Backspace
            if (cursorX > 0) {
                buffer[cursorY].erase(cursorX - 1, 1);
                cursorX--;
            } else if (cursorY > 0) {
                cursorX = buffer[cursorY - 1].length();
                buffer[cursorY - 1] += buffer[cursorY];
                buffer.erase(buffer.begin() + cursorY);
                cursorY--;
            }
        } else if (ch >= 32 && ch <= 126) { // Printable characters only
            buffer[cursorY].insert(cursorX, 1, ch);
            cursorX++;
        }
        isModified = true;
        needsRedraw = true;
    }

    void run() {
        while (true) {
            refreshScreen();
            
            // Add a small delay to prevent CPU overuse
            std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS

            if (_kbhit()) {
                int ch = _getch();
                if (ch == 0 || ch == 224) { // Special keys
                    ch = _getch();
                    switch (ch) {
                        case 72: // Up arrow
                            if (cursorY > 0) {
                                cursorY--;
                                cursorX = std::min(cursorX, buffer[cursorY].length());
                                needsRedraw = true;
                            }
                            break;
                        case 80: // Down arrow
                            if (cursorY < buffer.size() - 1) {
                                cursorY++;
                                cursorX = std::min(cursorX, buffer[cursorY].length());
                                needsRedraw = true;
                            }
                            break;
                        case 75: // Left arrow
                            if (cursorX > 0) {
                                cursorX--;
                                needsRedraw = true;
                            }
                            break;
                        case 77: // Right arrow
                            if (cursorX < buffer[cursorY].length()) {
                                cursorX++;
                                needsRedraw = true;
                            }
                            break;
                    }
                } else if (ch == 19) { // Ctrl+S
                    saveFile();
                } else if (ch == 17) { // Ctrl+Q
                    if (!isModified || MessageBoxW(NULL, L"Quit without saving?", L"Warning", MB_YESNO) == IDYES) {
                        break;
                    }
                } else {
                    insertChar(ch);
                }
            }
        }
    }
};

int main() {
    TextEditor editor;
    editor.run();
    return 0;
}