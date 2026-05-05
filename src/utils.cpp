# ifndef UTILS_CPP_FILES__
# define UTILS_CPP_FILES__

# include <iostream>
# include <cstdlib>
# include <random>
# include <thread>
# include <chrono>

std::string _RED = "\033[31m";
std::string _GREEN = "\033[32m";
std::string _YELLOW = "\033[33m";
std::string _BLUE = "\033[34m";
std::string _PURPLE = "\033[35m";
std::string _CYAN = "\033[36m";
std::string _WHITE = "\033[37m";

std::string _PURPLE_BG = "\033[45m";
std::string _WHITE_BG = "\033[47m";

std::string RESET = "\033[0m";

# if defined(_WIN32) || defined(_WIN64)
    # include <conio.h>
    # include <windows.h>
# else
    # include "include/conio_linux_port.h"
# endif

void reset_cursor(){
    std::cout << "\033[H";
}

void clear(){
    # if defined(_WIN32) || defined(_WIN64)
        std::system("cls");
    # else
        std::system("clear");
    # endif
}

char get_char(){
    # if defined(_WIN32) || defined(_WIN64)
        return getch();

    # else
        Console c;
        return c.getch();

    # endif
}

int random_number(int begin, int end){
    std::random_device rd;
    std::mt19937 generator(rd());

    std::uniform_int_distribution<> distrib(begin, end);
    return distrib(generator);
}

void sleep_for(int delay_in_ms){
    std::this_thread::sleep_for(std::chrono::milliseconds(delay_in_ms));
}

void slow_print(std::string message, int delay_in_ms = 50){
    for (auto ch : message){
        std::cout << ch << std::flush;
        sleep_for(delay_in_ms);
    }
}

void fix_mojibake_for_windows(){
    # if defined(_WIN32) || defined(_WIN64)
        SetConsoleOutputCP(CP_UTF8);

        HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD dwMode = 0;
        GetConsoleMode(hOut, &dwMode);
        dwMode |= 0x0004;
        SetConsoleMode(hOut, dwMode);
    # endif
}

// int main(){
//     # include <string>

//     std::string you_won = R"(
// █▄█ █▀█ █░█   █░█░█ █ █▄░█ █
// ░█░ █▄█ █▄█   ▀▄▀▄▀ █ █░▀█ ▄
// )";
//     slow_print(_GREEN + you_won + RESET, 15);
//     slow_print("         \n");

// }

# endif