# include <iostream>
# include <cstdlib>

# if defined(_WIN32) || defined(_WIN64)
    # include <conio.h>
# else
    # include "include/conio.h"
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