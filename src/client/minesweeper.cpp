# ifndef MINESWEEPER_CLI_
# define MINESWEEPER_CLI_

# include <iostream>
# include <vector>
# include <cstdlib>
# include <algorithm>
# include <iterator>
# include <string>
# include <atomic>
# include <thread>

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

// TODO: sound support

class Minesweeper{
protected:
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

    std::atomic<bool> is_game_over = false;
    bool player_won = false;
    bool gen_bombs = true;
    bool reveal_bomb_cells = false;
    std::atomic<bool> run_time_calc_thread;
    std::atomic<int> time_spent_in_seconds;

    // for multisweeper
    // std::vector<std::pair<int, int>> bomb_locations;
    std::vector<std::pair<int, int>> marked_bombs;
    std::atomic<bool> placing_bombs = false;
    std::atomic<bool> player_ready_to_start = false;
    bool multiplayer_gamemode = false;
    int player_penalty = 30; // seconds added for stepping in bomb
    int total_mul_bombs = 14;
    bool give_penalty = false;

    int board_size[2];
    int cursor_coords[2];
    int total_bombs;
    int remaining_flags;
    int cursor_radius; // only used to generate bombs

    int get_elem_at_cursor();
    void set_elem_at_cursor(int elem);

    void generate_bombs();
    void calculate_time();
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

    time_spent_in_seconds.store(0);
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

void Minesweeper::calculate_time() {
    while (run_time_calc_thread.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        int prev_time = time_spent_in_seconds.load();
        time_spent_in_seconds.store(prev_time + 1);
    }
}

void Minesweeper::display_board(){
    reset_cursor();

    if (placing_bombs.load())
        std::cout << _YELLOW << "[!] You have 60 seconds to place 14 bombs in the given board!\n                                                                        \n" << RESET;

    else if (multiplayer_gamemode)
        std::cout << _YELLOW << "[!] First one to clear the board wins.\n[!] Stepping on a bomb will result in a penalty of 30 seconds.\n\n" << RESET;

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

                    if (cursor_coords[0] == j and cursor_coords[1] == i)
                        std::cout << _CYAN + _PURPLE_BG + cursor;

                    else if(board[i][j] == bomb_cell or board[i][j] == bomb_cell + flag_addn){
                        std::pair<int, int> coords = {j, i};
                        bool find_marked_bomb = std::find(marked_bombs.begin(), marked_bombs.end(), coords) != marked_bombs.end();
                        if (reveal_bomb_cells or placing_bombs.load() or find_marked_bomb)
                            std::cout << _RED + "✸";
                        
                        else if (board[i][j] == bomb_cell + flag_addn and not placing_bombs.load())
                            std::cout << _RED + "▶";

                        else std::cout << _GREEN + "█";
                    }

                    else if(std::find(std::begin(cell_lists), std::end(cell_lists), board[i][j]) != std::end(cell_lists) and (not placing_bombs.load()))
                        std::cout << _RED + "▶";

                    else if(board[i][j] > numbered_cell and (not placing_bombs.load())){
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

    if (not placing_bombs.load())
        std::cout << "Remaining Flags: " << remaining_flags << "              \n";
    else
        std::cout << "Remaining Bombs: " << remaining_flags << "              \n";
    std::cout << "Time Spent: " << time_spent_in_seconds.load() << " seconds                   \n";

    std::cout << "\nCursor coordinates: (" << cursor_coords[0] + 1 << ", " << cursor_coords[1] + 1 << ")                \n";
    if (not placing_bombs.load())
        std::cout << "WASD: Move, Q: Reveal Tile, E: Place/Remove Flag, P: Exit Game                         \n";
    else
        std::cout << "WASD: Move, Q: Place bomb in tile, E: Remove bomb from tile, T: Ready                  \n";
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
    } else if (input == 'q' and not placing_bombs.load()){
        if (gen_bombs){
            gen_bombs = false;
            generate_bombs();
        }

        int cursor_elem = get_elem_at_cursor();

        if (cursor_elem == bomb_cell){
            if (not multiplayer_gamemode) {
                is_game_over.store(true);
                player_won = false;
            } else {
                std::pair<int, int> current_coords = {cursor_coords[0], cursor_coords[1]};

                if (std::find(marked_bombs.begin(), marked_bombs.end(), current_coords) == marked_bombs.end()){
                    marked_bombs.push_back(current_coords);
                    int prev_time = time_spent_in_seconds.load();
                    time_spent_in_seconds.store(prev_time + player_penalty);
                }
            }
        }
        else if (std::find(std::begin(cell_lists), std::end(cell_lists), get_elem_at_cursor()) != std::end(cell_lists)) return;
        else if (cursor_elem == tile_cell)
            empty_out_tiles(cursor_coords[0], cursor_coords[1]);
    } else if (input == 'e' and not placing_bombs.load()){
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
    } else if (input == 'p' and not multiplayer_gamemode) std::exit(0);

    // placing bombs
    else if (input == 'q' and placing_bombs.load()) {
        if (get_elem_at_cursor() == bomb_cell) return;
        if (remaining_flags == 0) return;

        set_elem_at_cursor(bomb_cell);
        bomb_locations.push_back({cursor_coords[0], cursor_coords[1]});
        remaining_flags--;
    } else if (input == 'e' and placing_bombs.load()) {
        if (get_elem_at_cursor() != bomb_cell) return;
        if (remaining_flags == total_mul_bombs) return;

        set_elem_at_cursor(tile_cell);

        std::pair cursor_location = {cursor_coords[0], cursor_coords[1]};
        auto bomb_loc_in_vector = std::find(bomb_locations.begin(), bomb_locations.end(), cursor_location);
        bomb_locations.erase(bomb_loc_in_vector);

        remaining_flags++;
    } else if (input == 't' and placing_bombs.load() and remaining_flags == 0) player_ready_to_start.store(true); 
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

    is_game_over.store(true);
    player_won = true;
}

void Minesweeper::game_over_animation(){
    if (player_won){
        std::string you_won;

        if (not multiplayer_gamemode){
        you_won = R"(                                                                     
█▄█ █▀█ █░█   █░█░█ █ █▄░█ █                                        
░█░ █▄█ █▄█   ▀▄▀▄▀ █ █░▀█ ▄                                        
You cleared out all the bombs!                                        
)";
        } else {
            you_won = R"(                                                                     
█▄█ █▀█ █░█   █░█░█ █ █▄░█ █                                        
░█░ █▄█ █▄█   ▀▄▀▄▀ █ █░▀█ ▄                                        
You completed before your opponent!                                        
)";
        }

        display_board();
        slow_print(_GREEN + you_won + RESET, 15);
        sleep_for(500);
    } else {
        std::string you_lose;
        reveal_bomb_cells = true;

        if (not multiplayer_gamemode) {
        you_lose = R"(                                                                     
█▄█ █▀█ █░█   █░░ █▀█ █▀ █▀▀   ▀ █▀▀                                        
░█░ █▄█ █▄█   █▄▄ █▄█ ▄█ ██▄   ▄ █▄▄                                        
You Stepped on a bomb!                                        
)";
        } else {
            you_lose = R"(                                                                     
█▄█ █▀█ █░█   █░░ █▀█ █▀ █▀▀   ▀ █▀▀                                        
░█░ █▄█ █▄█   █▄▄ █▄█ ▄█ ██▄   ▄ █▄▄                                        
You failed to complete before your opponent!                                        
)";
        }

        display_board();
        slow_print(_RED + you_lose + RESET, 15);
        sleep_for(500);
    }

    run_time_calc_thread.store(false);
}

void Minesweeper::run(){
    run_time_calc_thread.store(true);

    std::thread time_calc_thread(&Minesweeper::calculate_time, this);
    time_calc_thread.detach();

    while(true){
        display_board();
        get_kb_input();

        check_for_win();
        if (is_game_over.load()){
            game_over_animation();
            break;
        }
    }
}

# endif