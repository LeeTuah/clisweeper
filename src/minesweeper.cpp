# include <iostream>
# include <vector>
# include <cstdlib>
# include <algorithm>
# include <iterator>
# include <string>

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

// TODO: complete the winning logic
// TODO: multiplayer

class Minesweeper{
private:
    std::vector<std::vector<int>> board;
    std::vector<std::pair<int, int>> bomb_locations;
    int difficulty; // 1 = easy (12x8, 11), 2 = normal (20x12, 40), 3 = hard (24x21, 99)

    char cursor = '^';

    int empty_cell = 10;     // ground touched and no bomb
    int tile_cell = 20;      // ground not touched yet
    int numbered_cell = 100; // tiles with numbers (1 to 8) eg. 101 == 1, 105 == 5
    int bomb_cell = -10;     // bomb is present
    int flag_addn = 1;       // adds by given num to a cell if flag places there
    int cell_lists[3] = {empty_cell + flag_addn, tile_cell + flag_addn, bomb_cell + flag_addn}; // ONLY FOR ITERATING

    bool is_game_over = false;
    bool player_won = false;
    bool gen_bombs = true;
    bool reveal_bomb_cells = false;

    int board_size[2];
    int cursor_coords[2];
    int total_bombs;
    int remaining_flags;
    int cursor_radius; // only used to generate bombs

    int get_elem_at_cursor();
    void set_elem_at_cursor(int elem);

    void generate_bombs();
    void display_board();
    void get_kb_input();
    void empty_out_tiles(int x, int y);
    void check_for_win();
    void game_over_animation();

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
        cursor_radius = 1;
    } else if (difficulty == 2) {
        board_size[0] = 20;
        board_size[1] = 12;
        total_bombs = 39;
        cursor_radius = 3;
    } else if (difficulty == 3) {
        board_size[0] = 28;
        board_size[1] = 18;
        total_bombs = 99;
        cursor_radius = 5;
    }

    cursor_coords[0] = (board_size[0] / 2) - 1;
    cursor_coords[1] = (board_size[1] / 2) - 1;
    remaining_flags = total_bombs;

    for (int i = 0; i < board_size[1]; i++){
        board.push_back(std::vector<int>());
        for (int j = 0; j < board_size[0]; j++){
            board[i].push_back(tile_cell);
        }
    }
}

int Minesweeper::get_elem_at_cursor(){
    return board[cursor_coords[1]][cursor_coords[0]];
}

void Minesweeper::set_elem_at_cursor(int elem){
    board[cursor_coords[1]][cursor_coords[0]] = elem;
}

void Minesweeper::generate_bombs(){
    int bombs_added = 0;

    std::vector<int> rel_tiles_near_cursor_x;
    std::vector<int> rel_tiles_near_cursor_y;

    for (int i = -cursor_radius; i <= cursor_radius; i++){
        rel_tiles_near_cursor_x.push_back(cursor_coords[0] + i);
        rel_tiles_near_cursor_y.push_back(cursor_coords[1] + i);
    }

    while (bombs_added < total_bombs){
        int x, y, dist_x, dist_y;
        x = random_number(0, board_size[0] - 1);
        y = random_number(0, board_size[1] - 1);

        if (board[y][x] == bomb_cell) continue;

        dist_x = std::abs(x - cursor_coords[0]);
        dist_y = std::abs(y - cursor_coords[1]);

        if (dist_x > cursor_radius or dist_y > cursor_radius){
            board[y][x] = bomb_cell;
            bomb_locations.push_back(std::pair<int, int>(x, y));
            bombs_added++;
        }
    }
}

void Minesweeper::display_board(){
    reset_cursor();

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

                    if((board[i][j] == bomb_cell or board[i][j] == bomb_cell + flag_addn) and reveal_bomb_cells) std::cout << _RED + "✸";
                    else if (cursor_coords[0] == j and cursor_coords[1] == i) std::cout << _CYAN + _PURPLE_BG + cursor;
                    else if(std::find(std::begin(cell_lists), std::end(cell_lists), board[i][j]) != std::end(cell_lists))
                        std::cout << _RED + "▶";

                    else if(board[i][j] > numbered_cell){
                        if (board[i][j] == numbered_cell + 1) std::cout << _CYAN;
                        else if (board[i][j] == numbered_cell + 2) std::cout << _YELLOW;
                        else if (board[i][j] >= numbered_cell + 3) std::cout << _RED;

                        std::cout << board[i][j] - numbered_cell;
                    }
                    else if(board[i][j] == empty_cell) std::cout << " ";
                    else std::cout << _GREEN + "█";

                    std::cout << RESET + " ┃";
                }

                else std::cout << "━━━╋";
            } std::cout << "   " << std::endl;
        }
    }

    std::cout << "Remaining Flags: " << remaining_flags << "              \n";
    std::cout << "Time Spent: " << "something something                   \n"; // TODO: stopwatch

    std::cout << "\nCursor coordinates: (" << cursor_coords[0] + 1 << ", " << cursor_coords[1] + 1 << ")                \n";
    std::cout << "WASD: Move, Q: Reveal Tile, E: Place/Remove Flag, P: Exit Game\n                         ";
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
        if (gen_bombs){
            gen_bombs = false;
            generate_bombs();
        }

        int cursor_elem = get_elem_at_cursor();

        if (cursor_elem == bomb_cell){
            is_game_over = true;
            player_won = false;
        }
        else if (std::find(std::begin(cell_lists), std::end(cell_lists), get_elem_at_cursor()) != std::end(cell_lists)) return;
        else if (cursor_elem == tile_cell)
            empty_out_tiles(cursor_coords[0], cursor_coords[1]);
    } else if (input == 'e'){
        if (std::find(std::begin(cell_lists), std::end(cell_lists), get_elem_at_cursor()) != std::end(cell_lists)){
            set_elem_at_cursor(get_elem_at_cursor() - flag_addn);
            remaining_flags++;
        }

        else{ // place flag
            if (remaining_flags > 0 and get_elem_at_cursor() < numbered_cell){
                set_elem_at_cursor(get_elem_at_cursor() + flag_addn);
                remaining_flags--;
            }
        }
    } else if (input == 'p') std::exit(0);
}

void Minesweeper::empty_out_tiles(int x, int y){
    if ((x < 0 or x >= board_size[0]) or (y < 0 or y >= board_size[1])) return;

    if (board[y][x] == bomb_cell or board[y][x] == bomb_cell + flag_addn) return;

    // check for flags
    if (std::find(std::begin(cell_lists), std::end(cell_lists), board[y][x]) != std::end(cell_lists)) return;

    if (board[y][x] != tile_cell) return;

    int neighbor_bomb_count = 0;
    for (int i = -1; i <= 1; i++){
        for (int j = -1; j <= 1; j++){
            int dx = x + j;
            int dy = y + i;

            if (i == 0 and j == 0) continue;
            if ((dx < 0 or dx >= board_size[0]) or (dy < 0 or dy >= board_size[1])) continue;

            if (board[dy][dx] == bomb_cell or board[dy][dx] == bomb_cell + flag_addn) neighbor_bomb_count++;
        }
    }

    if (neighbor_bomb_count > 0){
        board[y][x] = numbered_cell + neighbor_bomb_count;
        return;
    }

    for (int i = -1; i <= 1; i++){
        for (int j = -1; j <= 1; j++){
            int dx = x + j;
            int dy = y + i;

            if (i == 0 and j == 0) continue;
            if ((dx < 0 or dx >= board_size[0]) or (dy < 0 or dy >= board_size[1])) continue;

            board[y][x] = empty_cell;
            empty_out_tiles(dx, dy);
        }
    }
}

void Minesweeper::check_for_win(){
    for (int i = 0; i < board_size[1]; i++){
        for (int j = 0; j < board_size[0]; j++){
            int e = board[i][j];

            if(not (e == empty_cell or e == bomb_cell or e == bomb_cell + flag_addn or e > numbered_cell)) return;
        }
    }

    is_game_over = true;
    player_won = true;
}

void Minesweeper::game_over_animation(){
    if (player_won){
        std::string you_won = R"(
█▄█ █▀█ █░█   █░█░█ █ █▄░█ █
░█░ █▄█ █▄█   ▀▄▀▄▀ █ █░▀█ ▄
You cleared out all the bombs!
)";

        display_board();
        slow_print(_GREEN + you_won + RESET, 15);
        sleep_for(500);
    } else {
        reveal_bomb_cells = true;
        std::string you_lose = R"(
█▄█ █▀█ █░█   █░░ █▀█ █▀ █▀▀   ▀ █▀▀
░█░ █▄█ █▄█   █▄▄ █▄█ ▄█ ██▄   ▄ █▄▄
You Stepped on a bomb!
)";

        display_board();
        slow_print(_RED + you_lose + RESET, 15);
        sleep_for(500);
    }
}

void Minesweeper::run(){
    while(true){
        display_board();
        get_kb_input();

        check_for_win();
        if (is_game_over){
            game_over_animation();
            break;
        }
    }
}

// int main(int argc, char** argv){
//     clear();
//     fix_mojibake_for_windows();

//     int difficulty = 1;

//     Minesweeper minesweeper(difficulty);
//     minesweeper.run();

//     return 0;
// }