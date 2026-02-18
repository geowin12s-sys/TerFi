#include <iostream>
#include <string>
#include <filesystem>
#include <windows.h>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <ctime>

namespace fs = std::filesystem;

// ==================== ЦВЕТА ====================
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

void setColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void resetColor() {
    setColor(WHITE);
}

void clearScreen() {
    system("cls");
}

// ==================== ФУНКЦИИ ====================

// Форматирование размера файла (байты -> КБ, МБ, ГБ)
std::string formatSize(uintmax_t size) {
    const char* units[] = {"Б", "КБ", "МБ", "ГБ"};
    int unitIndex = 0;
    double doubleSize = size;

    while (doubleSize >= 1024 && unitIndex < 3) {
        doubleSize /= 1024;
        unitIndex++;
    }

    char buffer[50];
    if (unitIndex == 0) {
        sprintf(buffer, "%llu %s", size, units[unitIndex]);
    } else {
        sprintf(buffer, "%.2f %s", doubleSize, units[unitIndex]);
    }
    return std::string(buffer);
}

// САМЫЙ ПРОСТОЙ ВАРИАНТ - без даты
std::string formatTime(const fs::file_time_type& ftime) {
    return "--/--/---- --:--";
}

// Показать помощь
void showHelp() {
    setColor(CYAN);
    std::cout << "\n=================== СПРАВКА ===================\n";
    setColor(YELLOW);
    std::cout << "📁 НАВИГАЦИЯ:\n";
    setColor(WHITE);
    std::cout << "  <имя папки>    - войти в папку\n";
    std::cout << "  ..              - вернуться назад\n";
    std::cout << "  ~               - перейти в домашнюю папку\n";
    std::cout << "  /               - перейти в корень диска\n";

    setColor(YELLOW);
    std::cout << "\n📄 КОМАНДЫ:\n";
    setColor(WHITE);
    std::cout << "  copy <файл> <путь>   - копировать файл\n";
    std::cout << "  move <файл> <путь>   - переместить файл\n";
    std::cout << "  del <файл>           - удалить файл\n";
    std::cout << "  mkdir <имя>          - создать папку\n";
    std::cout << "  rename <старое> <новое> - переименовать\n";

    setColor(YELLOW);
    std::cout << "\n🔧 НАСТРОЙКИ:\n";
    setColor(WHITE);
    std::cout << "  sort name             - сортировать по имени\n";
    std::cout << "  sort size             - сортировать по размеру\n";
    std::cout << "  sort date             - сортировать по дате\n";
    std::cout << "  sort type             - сортировать по типу\n";
    std::cout << "  show hidden           - показать скрытые файлы\n";
    std::cout << "  hide hidden           - скрыть скрытые файлы\n";

    setColor(YELLOW);
    std::cout << "\n🎨 ПРОЧЕЕ:\n";
    setColor(WHITE);
    std::cout << "  clear           - очистить экран\n";
    std::cout << "  help            - показать эту справку\n";
    std::cout << "  exit / q        - выйти\n";
    setColor(CYAN);
    std::cout << "==============================================\n\n";
    resetColor();
}

// Структура для элемента (файл или папка)
struct FileItem {
    fs::path path;
    std::string name;
    bool isDirectory;
    uintmax_t size;
    fs::file_time_type lastWriteTime;
    std::string extension;
};

// Получить список файлов с сортировкой
std::vector<FileItem> getFileList(const fs::path& directory, const std::string& sortBy, bool showHidden) {
    std::vector<FileItem> items;

    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            // Пропускаем скрытые если надо
            if (!showHidden) {
                std::string filename = entry.path().filename().string();
                if (!filename.empty() && filename[0] == '.') continue;
            }

            FileItem item;
            item.path = entry.path();
            item.name = entry.path().filename().string();
            item.isDirectory = entry.is_directory();
            item.lastWriteTime = entry.exists() ? fs::last_write_time(entry) : fs::file_time_type::min();

            if (item.isDirectory) {
                item.size = 0;  // для папок размер не считаем
                item.extension = "<DIR>";
            } else {
                item.size = entry.file_size();
                item.extension = entry.path().extension().string();
                if (item.extension.empty()) item.extension = "<ФАЙЛ>";
            }

            items.push_back(item);
        }
    } catch (...) {
        // Игнорируем ошибки доступа
    }

    // Сортировка
    if (sortBy == "name") {
        std::sort(items.begin(), items.end(), [](const FileItem& a, const FileItem& b) {
            return a.name < b.name;
        });
    } else if (sortBy == "size") {
        std::sort(items.begin(), items.end(), [](const FileItem& a, const FileItem& b) {
            if (a.isDirectory && !b.isDirectory) return true;  // папки выше
            if (!a.isDirectory && b.isDirectory) return false;
            return a.size > b.size;
        });
    } else if (sortBy == "date") {
        std::sort(items.begin(), items.end(), [](const FileItem& a, const FileItem& b) {
            return a.lastWriteTime > b.lastWriteTime;
        });
    } else if (sortBy == "type") {
        std::sort(items.begin(), items.end(), [](const FileItem& a, const FileItem& b) {
            if (a.isDirectory && !b.isDirectory) return true;
            if (!a.isDirectory && b.isDirectory) return false;
            return a.extension < b.extension;
        });
    }

    return items;
}

// Копировать файл
bool copyFile(const fs::path& source, const std::string& destStr) {
    try {
        fs::path dest = destStr;
        if (!dest.is_absolute()) {
            dest = fs::current_path() / dest;
        }

        if (fs::exists(source) && !fs::is_directory(source)) {
            fs::copy(source, dest, fs::copy_options::overwrite_existing);
            return true;
        }
    } catch (...) {}
    return false;
}

// Переместить/переименовать файл
bool moveFile(const fs::path& source, const std::string& destStr) {
    try {
        fs::path dest = destStr;
        if (!dest.is_absolute()) {
            dest = fs::current_path() / dest;
        }

        if (fs::exists(source)) {
            fs::rename(source, dest);
            return true;
        }
    } catch (...) {}
    return false;
}

// Удалить файл/папку
bool deleteFile(const std::string& name) {
    try {
        fs::path target = fs::current_path() / name;
        if (fs::exists(target)) {
            if (fs::is_directory(target)) {
                return fs::remove_all(target) > 0;
            } else {
                return fs::remove(target);
            }
        }
    } catch (...) {}
    return false;
}

// Создать папку
bool createDirectory(const std::string& name) {
    try {
        fs::path newDir = fs::current_path() / name;
        return fs::create_directory(newDir);
    } catch (...) {}
    return false;
}

// ==================== ОСНОВНАЯ ФУНКЦИЯ ====================

int main() {
    system("chcp 65001 > nul");  // русский язык

    fs::path current_path = fs::current_path();
    std::string command;
    std::string sortBy = "name";
    bool showHidden = false;

    while (true) {
        clearScreen();

        // Шапка
        setColor(CYAN);
        std::cout << "╔══════════════════════════════════════════════════════════╗\n";
        setColor(YELLOW);
        std::cout << "║           CONSOLE COMMANDER v1.0 - ПОЛНЫЙ ФАРШ          ║\n";
        setColor(CYAN);
        std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
        resetColor();

        // Текущий путь
        setColor(DARK_GRAY);
        std::cout << "📍 ";
        setColor(WHITE);
        std::cout << "Текущая папка: ";
        setColor(GREEN);
        std::cout << current_path << "\n";
        resetColor();

        // Инфо о сортировке
        setColor(DARK_GRAY);
        std::cout << "📊 Сортировка: " << sortBy;
        if (showHidden) std::cout << " | Показывать скрытые";
        std::cout << "\n\n";
        resetColor();

        // Заголовок таблицы
        setColor(CYAN);
        std::cout << "┌──────┬──────────────────────────────────┬────────────┬─────────────────┐\n";
        std::cout << "│ Тип  │ Имя                              │ Размер     │ Дата изменения │\n";
        std::cout << "├──────┼──────────────────────────────────┼────────────┼─────────────────┤\n";
        resetColor();

        // Получаем и выводим файлы
        auto items = getFileList(current_path, sortBy, showHidden);

        for (const auto& item : items) {
            // Тип и цвет
            if (item.isDirectory) {
                setColor(GREEN);
                std::cout << "│ 📁   │ ";
                resetColor();
            } else {
                // Цвет в зависимости от расширения
                if (item.extension == ".exe" || item.extension == ".bat") {
                    setColor(RED);
                } else if (item.extension == ".cpp" || item.extension == ".h" || item.extension == ".py") {
                    setColor(CYAN);
                } else if (item.extension == ".txt" || item.extension == ".md") {
                    setColor(WHITE);
                } else if (item.extension == ".jpg" || item.extension == ".png" || item.extension == ".gif") {
                    setColor(MAGENTA);
                } else {
                    setColor(LIGHT_GRAY);
                }
                std::cout << "│ 📄   │ ";
                resetColor();
            }

            // Имя (обрезаем если длинное)
            std::string displayName = item.name;
            if (displayName.length() > 30) {
                displayName = displayName.substr(0, 27) + "...";
            }
            std::cout << std::left << std::setw(32) << displayName;

            // Размер
            setColor(DARK_GRAY);
            std::cout << " │ ";
            resetColor();

            if (item.isDirectory) {
                setColor(GREEN);
                std::cout << std::right << std::setw(10) << "<ПАПКА>";
                resetColor();
            } else {
                setColor(YELLOW);
                std::cout << std::right << std::setw(10) << formatSize(item.size);
                resetColor();
            }

            // Дата
            setColor(DARK_GRAY);
            std::cout << " │ ";
            resetColor();

            try {
                std::cout << formatTime(item.lastWriteTime);
            } catch (...) {
                std::cout << "     неизвестно     ";
            }

            std::cout << " │\n";
        }

        // Нижняя граница таблицы
        setColor(CYAN);
        std::cout << "└──────┴──────────────────────────────────┴────────────┴─────────────────┘\n";
        resetColor();

        // Подсказка
        setColor(DARK_GRAY);
        std::cout << "\n💡 'help' — список команд, 'exit' — выход\n";
        resetColor();

        // Ввод команды
        setColor(CYAN);
        std::cout << "\n> ";
        resetColor();

        std::getline(std::cin, command);

        // ========== ОБРАБОТКА КОМАНД ==========

        if (command == "exit" || command == "q") {
            setColor(GREEN);
            std::cout << "\n👋 Пока! Заходи ещё!\n";
            resetColor();
            break;
        }
        else if (command == "help") {
            showHelp();
            std::cout << "Нажми Enter чтобы продолжить...";
            std::cin.get();
        }
        else if (command == "clear") {
            // просто очистится в начале цикла
        }
        else if (command == "..") {
            if (current_path.has_parent_path()) {
                current_path = current_path.parent_path();
            } else {
                setColor(RED);
                std::cout << "\n❌ Уже в корне!\n";
                resetColor();
                Sleep(1000);
            }
        }
        else if (command == "~") {
            try {
                current_path = fs::path(getenv("USERPROFILE"));
            } catch (...) {
                setColor(RED);
                std::cout << "\n❌ Не могу найти домашнюю папку\n";
                resetColor();
                Sleep(1000);
            }
        }
        else if (command == "/") {
            try {
                current_path = fs::path(current_path.root_path());
            } catch (...) {
                setColor(RED);
                std::cout << "\n❌ Ошибка\n";
                resetColor();
                Sleep(1000);
            }
        }
        else if (command.substr(0, 4) == "sort") {
            if (command.length() > 5) {
                std::string sortType = command.substr(5);
                if (sortType == "name" || sortType == "size" || sortType == "date" || sortType == "type") {
                    sortBy = sortType;
                    setColor(GREEN);
                    std::cout << "\n✅ Сортировка изменена на " << sortType << "\n";
                    resetColor();
                    Sleep(800);
                } else {
                    setColor(RED);
                    std::cout << "\n❌ Неизвестный тип сортировки\n";
                    resetColor();
                    Sleep(800);
                }
            }
        }
        else if (command == "show hidden") {
            showHidden = true;
            setColor(GREEN);
            std::cout << "\n✅ Показываю скрытые файлы\n";
            resetColor();
            Sleep(800);
        }
        else if (command == "hide hidden") {
            showHidden = false;
            setColor(GREEN);
            std::cout << "\n✅ Скрытые файлы скрыты\n";
            resetColor();
            Sleep(800);
        }
        else if (command.substr(0, 4) == "copy" && command.length() > 5) {
            size_t spacePos = command.find(' ', 5);
            if (spacePos != std::string::npos) {
                std::string source = command.substr(5, spacePos - 5);
                std::string dest = command.substr(spacePos + 1);

                if (copyFile(fs::current_path() / source, dest)) {
                    setColor(GREEN);
                    std::cout << "\n✅ Файл скопирован\n";
                } else {
                    setColor(RED);
                    std::cout << "\n❌ Ошибка копирования\n";
                }
                resetColor();
                Sleep(1000);
            }
        }
        else if (command.substr(0, 4) == "move" && command.length() > 5) {
            size_t spacePos = command.find(' ', 5);
            if (spacePos != std::string::npos) {
                std::string source = command.substr(5, spacePos - 5);
                std::string dest = command.substr(spacePos + 1);

                if (moveFile(fs::current_path() / source, dest)) {
                    setColor(GREEN);
                    std::cout << "\n✅ Файл перемещён\n";
                } else {
                    setColor(RED);
                    std::cout << "\n❌ Ошибка перемещения\n";
                }
                resetColor();
                Sleep(1000);
            }
        }
        else if (command.substr(0, 6) == "rename" && command.length() > 7) {
            size_t spacePos = command.find(' ', 7);
            if (spacePos != std::string::npos) {
                std::string oldName = command.substr(7, spacePos - 7);
                std::string newName = command.substr(spacePos + 1);

                if (moveFile(fs::current_path() / oldName, newName)) {
                    setColor(GREEN);
                    std::cout << "\n✅ Переименовано\n";
                } else {
                    setColor(RED);
                    std::cout << "\n❌ Ошибка переименования\n";
                }
                resetColor();
                Sleep(1000);
            }
        }
        else if (command.substr(0, 3) == "del" && command.length() > 4) {
            std::string target = command.substr(4);

            setColor(RED);
            std::cout << "⚠️  Точно удалить '" << target << "'? (y/n): ";
            resetColor();

            std::string confirm;
            std::getline(std::cin, confirm);

            if (confirm == "y" || confirm == "yes") {
                if (deleteFile(target)) {
                    setColor(GREEN);
                    std::cout << "✅ Удалено\n";
                } else {
                    setColor(RED);
                    std::cout << "❌ Ошибка удаления\n";
                }
                resetColor();
                Sleep(1000);
            }
        }
        else if (command.substr(0, 5) == "mkdir" && command.length() > 6) {
            std::string dirName = command.substr(6);

            if (createDirectory(dirName)) {
                setColor(GREEN);
                std::cout << "\n✅ Папка создана\n";
            } else {
                setColor(RED);
                std::cout << "\n❌ Ошибка создания\n";
            }
            resetColor();
            Sleep(1000);
        }
        else if (!command.empty()) {
            // Пробуем войти в папку
            fs::path new_path = current_path / command;

            if (fs::exists(new_path) && fs::is_directory(new_path)) {
                try {
                    current_path = fs::canonical(new_path);
                } catch (...) {
                    current_path = new_path;  // если canonical не сработал
                }
            } else {
                setColor(RED);
                std::cout << "\n❌ Неизвестная команда или папка '" << command << "'\n";
                resetColor();
                Sleep(1000);
            }
        }
    }

    return 0;
}
