# include <iostream>
# include <cstdlib>
# include <random>

std::string _RED = "\033[31m";
std::string _GREEN = "\033[32m";
std::string _YELLOW = "\033[33m";
std::string _BLUE = "\033[34m";
std::string _PURPLE = "\033[35m";
std::string _CYAN = "\033[36m";
std::string _WHITE = "\033[37m";

std::string RESET = "\033[0m";

# if defined(_WIN32) || defined(_WIN64)
    # include <conio.h>
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