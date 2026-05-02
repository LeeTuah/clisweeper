# include <iostream>
# include <cstdlib>

# include "include/conio.h"

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

int getcharr(){
    // # if defined(_WIN32) || defined(_WIN64)
    //     # include <Windows.h>

    //     if(GetKeyState('A') & 0x8000/*Check if high-order bit is set (1 << 15)*/)
    //     {
    //         // Do stuff
    //     }
    // # else

    // # endif

    Console c;

    return c.getch();
}

int main(int argc, char** argv){
    while(true){
        std::cout << (char)getcharr() << '\n';
    }
}