#include <iostream>
#include <string>
#include <filesystem>
#include <windows.h>  // для цветов

namespace fs = std::filesystem;

// Цвета (коды)
enum Color {
    BLACK = 0,
    DARK_BLUE = 1,
    DARK_GREEN = 2,
    DARK_CYAN = 3,
    DARK_RED = 4,
    DARK_MAGENTA = 5,
    DARK_YELLOW = 6,
    LIGHT_GRAY = 7,
    DARK_GRAY = 8,
    BLUE = 9,
    GREEN = 10,
    CYAN = 11,
    RED = 12,
    MAGENTA = 13,
    YELLOW = 14,
    WHITE = 15
};

// Функция для установки цвета
void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

// Функция для сброса цвета (белый)
void resetColor() {
    setColor(WHITE);
}

int main() {
    system("chcp 65001 > nul");  // русский язык
    
    // Делаем красивое цветное приветствие
    setColor(CYAN);
    std::cout << "========================================\n";
    setColor(YELLOW);
    std::cout << "         CONSOLE COMMANDER v0.2        \n";
    setColor(CYAN);
    std::cout << "========================================\n\n";
    resetColor();

    // Текущая папка
    fs::path current_path = fs::current_path();
    setColor(DARK_GRAY);
    std::cout << "📍 ";
    setColor(WHITE);
    std::cout << "Текущая папка: ";
    setColor(GREEN);
    std::cout << current_path << "\n\n";
    resetColor();

    // Заголовок таблицы
    setColor(CYAN);
    std::cout << "Содержимое:\n";
    setColor(DARK_GRAY);
    std::cout << "----------------------------------------\n";
    resetColor();

    // Выводим файлы и папки
    for (const auto& entry : fs::directory_iterator(current_path)) {
        if (entry.is_directory()) {
            setColor(GREEN);  // папки зелёные
            std::cout << "[📁] ";
            resetColor();
            std::cout << entry.path().filename().string() << "\n";
        } else {
            setColor(LIGHT_GRAY);  // файлы светло-серые
            std::cout << "[📄] ";
            resetColor();
            std::cout << entry.path().filename().string();
            
            setColor(DARK_GRAY);  // размер серым
            std::cout << " (" << entry.file_size() << " б)";
            resetColor();
            std::cout << "\n";
        }
    }

    setColor(DARK_GRAY);
    std::cout << "----------------------------------------\n";
    resetColor();
    
    setColor(YELLOW);
    std::cout << "\n✨ Нажми Enter, чтобы выйти... ✨";
    resetColor();
    std::cin.get();

    return 0;
}