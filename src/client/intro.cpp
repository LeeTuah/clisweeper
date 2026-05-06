# ifndef INTRO_CPP_
# define INTRO_CPP_

# include "utils.cpp"

std::string clisweeper = _GREEN + R"(
░█████╗░██╗░░░░░██╗  ░██████╗░██╗░░░░░░░██╗███████╗███████╗██████╗░███████╗██████╗░
██╔══██╗██║░░░░░██║  ██╔════╝░██║░░██╗░░██║██╔════╝██╔════╝██╔══██╗██╔════╝██╔══██╗
██║░░╚═╝██║░░░░░██║  ╚█████╗░░╚██╗████╗██╔╝█████╗░░█████╗░░██████╔╝█████╗░░██████╔╝
██║░░██╗██║░░░░░██║  ░╚═══██╗░░████╔═████║░██╔══╝░░██╔══╝░░██╔═══╝░██╔══╝░░██╔══██╗
╚█████╔╝███████╗██║  ██████╔╝░░╚██╔╝░╚██╔╝░███████╗███████╗██║░░░░░███████╗██║░░██║
░╚════╝░╚══════╝╚═╝  ╚═════╝░░░░╚═╝░░░╚═╝░░╚══════╝╚══════╝╚═╝░░░░░╚══════╝╚═╝░░╚═╝    
)" + RESET;

void intro_screen(){
    std::cout << _YELLOW + "It is recommended to fullscreen the terminal before starting the game.\n" + RESET;
    std::cout << "Press any key to continue.....";

    get_char();
    clear();

    slow_print(clisweeper, 3); // TODO: set it to 3 later
}

void controls(){
    std::cout << "Game Basics: \n";
    std::cout << "WASD -> Move, Q -> Reveal/Dig out a tile, E -> Place/Remove a flag\n";
    std::cout << "\nIn minesweeper, you have to reveal/dig out every tile/square in the given area, by avoiding the bombs.\n";
    std::cout << "Flags are provided to be placed over the bomb, as a visual indicator. Even if you do not have all the flags placed down,\n";
    std::cout << "you can still win by exposing every square except the ones with bombs.\n\n";
    std::cout << "You might see cells which have numbers labelled as 1, 2, 3, until 8. This means that that many bombs are present in the \n";
    std::cout << "neighboring cell of the numbered cell. For example, if a cell is numbered 3, it means that there are three bombs among the\n";
    std::cout << "eight neighboring tiles from the tile where the 3 is showing up. These will help you deduce the potential locations of\n";
    std::cout << "bombs.\n";
    
    std::cout << "Press any key to exit........";
    get_char();
}

# endif