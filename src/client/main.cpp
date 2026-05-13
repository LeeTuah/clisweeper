# include "multiplayer.cpp"
# include "intro.cpp"

# include "../include/Menu.h"

void singleplayer(){
    std::cout << _CYAN << "Choose your difficulty: \n";
    std::cout << _GREEN << "1. Easy \n2. Medium \n3. Hard \n4. Back \n>> " << RESET;

    char input = get_char();

    if (input == '1') {
        Minesweeper m(1);
        m.run();
    } else if (input == '2') {
        Minesweeper m(2);
        m.run();
    } else if (input == '3') {
        Minesweeper m(3);
        m.run();
    } else if (input == '4') {
        return;
    }
}

void multiplayer() {
    clear();

    Multisweeper m(1);
    m.run();
}

int main(){
    #ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
            std::cout << "WSAStartup failed.\n";
            return 1;
        }
    #endif


    fix_mojibake_for_windows();
    clear();

    intro_screen();

    Menu intro_menu;
    intro_menu.set_heading("CLI Sweeper");
    intro_menu.header = clisweeper;
    intro_menu.set_colors("32", "33");
    intro_menu.set_max_len(33);

    intro_menu.add_field("Singleplayer", singleplayer);
    intro_menu.add_field("Multiplayer", multiplayer);
    intro_menu.add_field("Controls", controls);

    intro_menu.run_menu();

    return 0;
}