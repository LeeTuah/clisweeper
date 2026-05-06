# include "minesweeper.cpp"
# include "intro.cpp"

# include "../include/Menu.h"

void temp(){
    // 
}

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

int main(){
    fix_mojibake_for_windows();
    clear();

    intro_screen();

    Menu intro_menu;
    intro_menu.set_heading("CLI Sweeper");
    intro_menu.header = clisweeper;
    intro_menu.set_colors("32", "33");
    intro_menu.set_max_len(33);

    intro_menu.add_field("Singleplayer", singleplayer);
    intro_menu.add_field("Multiplayer", temp);
    intro_menu.add_field("Controls", controls);

    intro_menu.run_menu();

    return 0;
}