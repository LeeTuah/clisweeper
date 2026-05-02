# include <iostream>
# include <vector>
# include <cstdlib>

# include "utils.cpp"

class Minesweeper{
private:
    std::vector<std::vector<int>> board;
    int difficulty; // 1 = easy (12x8, 11), 2 = normal (20x12, 40), 3 = hard (24x21, 99)

    char cursor = '^';
    
    int board_size[2];
    int cursor_coords[2];
    int total_bombs;

    void generate_board();
    void display_board();
    void get_kb_input();

public:
    Minesweeper(int difficulty);

    void run();
};

Minesweeper::Minesweeper(int difficulty){
    this->difficulty = difficulty;

    if(difficulty == 1){
        board_size[0] = 12;
        board_size[1] = 8;
        total_bombs = 11;
    } else if (difficulty == 2) {
        board_size[0] = 20;
        board_size[1] = 12;
        total_bombs = 39;
    } else if (difficulty == 3) { // TODO: change to a rect-shaped board size 
        board_size[0] = 24;
        board_size[1] = 21;
        total_bombs = 99;
    }

    cursor_coords[0] = (board_size[0] / 2) - 1;
    cursor_coords[1] = (board_size[1] / 2) - 1;
    this->generate_board();
}

void Minesweeper::generate_board(){
    for (int i = 0; i < board_size[1]; i++){
        board.push_back(std::vector<int>());
        for (int j = 0; j < board_size[0]; j++){
            board[i].push_back(0);
        }
    }


}

void Minesweeper::display_board(){
    clear();

    std::cout << "╋";
    for (int x = 0; x < board_size[0]; x++){
        std::cout << "━━━╋";
    } std::cout << std::endl;

    for (int i = 0; i < board_size[1]; i++){
        for (int x = 0; x < 2; x++){
            if (x == 0) std::cout << "┃";
            else std::cout << "╋";

            for (int j = 0; j < board_size[0]; j++){
                if (x == 0){
                    std::cout << " ";

                    if (cursor_coords[0] == j && cursor_coords[1] == i) std::cout << cursor;
                    else std::cout << " ";

                    std::cout << " ┃";
                }

                else std::cout << "━━━╋";
            } std::cout << std::endl;
        }
    }

    std::cout << "Cursor coordinates: (" << cursor_coords[0] << ", " << cursor_coords[1] << ")\n";
}

void Minesweeper::get_kb_input(){
    char input = get_char();

    if (input == 'w'){
        if (cursor_coords[1] > 0)
        cursor_coords[1] -= 1;
    } else if (input == 's'){
        if (cursor_coords[1] < board_size[1] - 1)
        cursor_coords[1] += 1;
    } else if (input == 'a'){
        if (cursor_coords[0] > 0)
        cursor_coords[0] -= 1;
    } else if (input == 'd'){
        if (cursor_coords[0] < board_size[0] - 1)
        cursor_coords[0] += 1;
    }
}

void Minesweeper::run(){
    while(true){
        display_board();
        get_kb_input();
    }
}

int main(int argc, char** argv){
    int difficulty = 1;

    Minesweeper minesweeper(difficulty);
    minesweeper.run();

    return 0;
}