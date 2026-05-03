# include <iostream>
# include <vector>
# include <cstdlib>
# include <algorithm>
# include <iterator>

# include "utils.cpp"

/*
Controls:

WASD -> Move
Q -> Reveal Tile
E -> Mark a flag

Symbols:
10 -> Empty Cell
20 -> Tile Cell
-10 -> Bomb Cell
(Cell below) + 1 -> Flag Placed
*/

class Minesweeper{
private:
    std::vector<std::vector<int>> board;
    int difficulty; // 1 = easy (12x8, 11), 2 = normal (20x12, 40), 3 = hard (24x21, 99)

    char cursor = '^';
    
    int empty_cell = 10;   // ground touched and no bomb
    int tile_cell = 20;    // ground not touched yet
    int bomb_cell = -10;   // bomb is present
    int flag_addn = 1;     // adds by given num to a cell if flag places there
    int cell_lists[3] = {empty_cell + flag_addn, tile_cell + flag_addn, bomb_cell + flag_addn}; // ONLY FOR ITERATING

    int board_size[2];
    int cursor_coords[2];
    int total_bombs;
    int remaining_flags;

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
        total_bombs = 14;
    } else if (difficulty == 2) {
        board_size[0] = 20;
        board_size[1] = 12;
        total_bombs = 39;
    } else if (difficulty == 3) { // TODO: change to a rect-shaped board size 
        board_size[0] = 28;
        board_size[1] = 18;
        total_bombs = 99;
    }

    cursor_coords[0] = (board_size[0] / 2) - 1;
    cursor_coords[1] = (board_size[1] / 2) - 1;
    remaining_flags = total_bombs;
    this->generate_board();
}

void Minesweeper::generate_board(){
    for (int i = 0; i < board_size[1]; i++){
        board.push_back(std::vector<int>());
        for (int j = 0; j < board_size[0]; j++){
            board[i].push_back(tile_cell);
        }
    }

    int bombs_added = 0;

    while (bombs_added < total_bombs){
        int x, y;
        x = random_number(0, board_size[0] - 1);
        y = random_number(0, board_size[1] - 1);

        if (board[y][x] != bomb_cell && (y != cursor_coords[1] && x != cursor_coords[0])){
            board[y][x] = bomb_cell;
            bombs_added++;
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

                    if (cursor_coords[0] == j && cursor_coords[1] == i) std::cout << _CYAN + cursor;

                    else if(std::find(std::begin(cell_lists), std::end(cell_lists), board[i][j]) != std::end(cell_lists))
                        std::cout << _GREEN + "▶";
                    
                    else if(board[i][j] == empty_cell) std::cout << " ";
                    else if(board[i][j] == bomb_cell) std::cout << "B";
                    else std::cout << _YELLOW + "█";

                    std::cout << RESET + " ┃";
                }

                else std::cout << "━━━╋";
            } std::cout << std::endl;
        }
    }

    std::cout << "Remaining Flags: " << remaining_flags << '\n';
    std::cout << "Time Spent: " << "something something\n"; // TODO: stopwatch

    std::cout << "\nCursor coordinates: (" << cursor_coords[0] + 1 << ", " << cursor_coords[1] + 1 << ")\n";
    std::cout << "WASD: Move, Q: Reveal Tile, E: Edit Flag\n";
    std::cout << std::endl;
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
    } else if (input == 'q'){
        // TODO: do something
    } else if (input == 'e'){
        if (std::find(std::begin(cell_lists), std::end(cell_lists), board[cursor_coords[1]][cursor_coords[0]]) != std::end(cell_lists)){
            board[cursor_coords[1]][cursor_coords[0]] -= flag_addn;
            remaining_flags++;
        }

        else{ // place flag
            if (remaining_flags > 0){
                board[cursor_coords[1]][cursor_coords[0]] += flag_addn;
                remaining_flags--;
            }
        }
    }
}

void Minesweeper::run(){
    while(true){
        display_board();
        get_kb_input();
    }
}

int main(int argc, char** argv){
    int difficulty = 3;

    Minesweeper minesweeper(difficulty);
    minesweeper.run();

    return 0;
}