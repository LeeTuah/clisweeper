# include <iostream>
# include <cstring>
# include <string>
# include <vector>
# include <cctype>
# include <atomic>

# include "minesweeper.cpp"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib") // Tells MSVC to link the socket library
    #define CLOSE_SOCKET closesocket
    typedef SOCKET SocketType; // Windows uses SOCKET
#else
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define CLOSE_SOCKET close
    typedef int SocketType; // Linux uses int
#endif

class Multisweeper : public Minesweeper{
protected:
    SocketType client_socket;
    sockaddr_in server_address;
    std::string name;

    std::atomic<bool> run_player_join_thread;
    std::atomic<bool> run_display_room_thread;
    std::atomic<bool> room_aborted;
    std::atomic<bool> room_started;
    std::vector<std::string> players_in_lobby;
    bool need_to_append_player;

    std::atomic<bool> is_host;
    std::atomic<bool> redraw_room_menu;
    std::atomic<int> time_spent_by_opp;

    std::string recv_msg();
    void check_for_player_joins();
    void check_for_opp_win();
    void display_room_message(std::string message = "");
    void run_game();
public:
    Multisweeper(int difficulty) : Minesweeper(difficulty){
        client_socket = socket(AF_INET, SOCK_STREAM, 0);

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(6741);
        inet_pton(AF_INET, "192.168.1.4", &(server_address.sin_addr));

        multiplayer_gamemode = true;
        time_spent_by_opp = 0;
    }

    void run();
};

std::string Multisweeper::recv_msg() {
    std::string message;
    char buffer[1024] = {0};

    int recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (recv_bytes <= 0) {
        std::cout << "Failed to connect to the server!\n";
        return "";
    } message = std::string(buffer, recv_bytes);

    return message;
}

void Multisweeper::check_for_player_joins() {
    while(run_player_join_thread.load()) {
        std::string message = recv_msg();
        std::string status_code = substr(message, 3); 

        if (status_code == "201") {
            std::string player = message.substr(4);
            players_in_lobby.push_back(player);

            redraw_room_menu.store(true);
        } else if (status_code == "202") {
            std::string player = message.substr(4);
            auto iter = std::find(players_in_lobby.begin(), players_in_lobby.end(), player);

            if (iter != players_in_lobby.end()) players_in_lobby.erase(iter);

            redraw_room_menu.store(true);
        } else if (status_code == "200") {
            break;
        } else if (status_code == "205") {
            room_aborted.store(true);
            redraw_room_menu.store(true);
            break;
        } else if (status_code == "670") {
            room_started.store(true);
            redraw_room_menu.store(true);
            break;
        }
    }
}

void Multisweeper::check_for_opp_win() {
    std::string msg = recv_msg(); // format: !win <time_spent_in_seconds>
    if (msg.length() == 0) return;

    std::string win_status = substr(msg, 4);
    std::string time_spent;

    if (win_status == "!win") {
        time_spent = msg.substr(5);
        time_spent_by_opp.store(std::stoi(time_spent));
    } else if (win_status == "!dis") { // client disconnected
        // do something
    }
}

void Multisweeper::display_room_message(std::string message /*= ""*/) {
    while (run_display_room_thread.load()) {
        if (redraw_room_menu.load()) {
            clear();

            if (is_host.load()) {
                std::cout << "Successfully generated a room!\n" << message << std::endl;
                std::cout << "\nPress \'W\' to start the match.\nPress \'S\' to abort the lobby.\nPress any key to refresh the board." << std::endl;
            } else {
                std::cout << "Successfully joined the room!\nWaiting for the host to start the game..." << std::endl;
                std::cout << "Press \'S\' to leave the lobby.\nPress any key to refresh the board." << std::endl;
            }
            
            std::cout << "\nPlayers in lobby:\n";
            for (auto iter : players_in_lobby) 
                std::cout << iter << std::endl;
            std::cout << std::endl;

            if (room_aborted.load())
                std::cout << "[!] The host has aborted the room!\n[!] Press any key to continue.....\n";

            else if (room_started.load())
                std::cout << "[!] The game has started!\n[!] Press any key to continue....\n";

            redraw_room_menu.store(false);
        }

        sleep_for(50);
    }
}

void Multisweeper::run_game() {
    run_time_calc_thread.store(true);
    std::thread time_calc_thread_prep(&Multisweeper::calculate_time, this);
    time_calc_thread_prep.detach();

    while (placing_bombs.load()) {
        display_board();
        get_kb_input();

        if (time_spent_in_seconds.load() >= 60) player_ready_to_start.store(true);

        if (player_ready_to_start.load()){
            run_time_calc_thread.store(false);
            break;
        }
    }
    placing_bombs.store(false);
    std::vector<std::pair<int, int>> opp_bomb_locations;

    for (int i = 0; i < board_size[1]; i++) {
        for (int j = 0; j < board_size[0]; j++) {
            board[i][j] = tile_cell;
        }
    }

    if (is_host.load()) {
        std::string message = "";
        for (auto coords : bomb_locations)
            message += std::to_string(coords.first) + "," + std::to_string(coords.second) + ",";
        send(client_socket, message.c_str(), message.length(), 0);

        message = recv_msg();
        std::vector<std::string> all_coords = split_string_to_vector(message);
        for (int i = 0; i < all_coords.size(); i += 2) {
            std::pair<int, int> coord_pair = {std::stoi(all_coords[i]), std::stoi(all_coords[i + 1])};
            opp_bomb_locations.push_back(coord_pair);
        }
    } else {
        std::string message = recv_msg();
        std::vector<std::string> all_coords = split_string_to_vector(message);
        for (int i = 0; i < all_coords.size(); i += 2) {
            std::pair<int, int> coord_pair = {std::stoi(all_coords[i]), std::stoi(all_coords[i + 1])};
            opp_bomb_locations.push_back(coord_pair);
        }

        message = "";
        for (auto coords : bomb_locations)
            message += std::to_string(coords.first) + "," + std::to_string(coords.second) + ",";
        send(client_socket, message.c_str(), message.length(), 0);
    }
    bomb_locations.clear();
    bomb_locations = opp_bomb_locations;
    for (auto bomb_coord : bomb_locations) board[bomb_coord.second][bomb_coord.first] = bomb_cell;

    remaining_flags = total_mul_bombs;

    run_time_calc_thread.store(true);
    std::thread time_calc_thread_main(&Multisweeper::calculate_time, this);
    time_calc_thread_main.detach();

    std::thread opp_win_check_thread(&Multisweeper::check_for_opp_win, this);
    opp_win_check_thread.detach();

    clear();
    time_spent_in_seconds.store(0);
    bool send_completed_time = true;

    while (true) {
        display_board();
        get_kb_input();

        check_for_win();
        if (is_game_over.load()) {
            if (send_completed_time) {
                std::string win_msg = "!win " + std::to_string(time_spent_in_seconds.load());
                send(client_socket, win_msg.c_str(), win_msg.length(), 0);
                send_completed_time = false;

                run_time_calc_thread.store(false);
            }

            if (time_spent_by_opp != 0) {
                player_won = (time_spent_in_seconds <= time_spent_by_opp); // FIXME draw when both have same time
                
                game_over_animation(); 
                break;
            }

            std::cout << "[I] Waiting for the other person to complete their game.\n[I] Press any key to refresh the board.\n" << std::endl;
        }
    }
}

void Multisweeper::run() {
    int connect_status = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connect_status == -1) {
        std::cout << "Failed to connect to the server!\n";
        return;
    }
    need_to_append_player = true;
    redraw_room_menu.store(true);
    is_host.store(false);
    room_aborted.store(false);
    room_started.store(false);
    placing_bombs.store(true);
    gen_bombs = false;

    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    send(client_socket, name.c_str(), name.length(), 0);

    std::string message = recv_msg();

    if (substr(message, 3) != "200") {
        std::cout << "An error occured!\n" << message;
        return;
    }

    while(true) {
        clear();

        std::cout << "Choose one of the following: \n";
        std::cout << "1. Host Game \n2. Join Game \n3. Exit\n>> ";

        run_player_join_thread.store(true);
        run_display_room_thread.store(true);
        char input = get_char();

        if (input == '1') {
            const char* msg = "!host";
            send(client_socket, msg, strlen(msg), 0);

            message = recv_msg();

            if (substr(message, 3) != "200") {
                std::cout << "An error occured!\n" << message;
                return;
            }
            message = message.substr(4);

            std::thread player_joins_thread(&Multisweeper::check_for_player_joins, this);
            player_joins_thread.detach();

            is_host.store(true);
            std::thread display_room_msg_thread(&Multisweeper::display_room_message, this, message);
            display_room_msg_thread.detach();

            while(true) {
                if (need_to_append_player){
                    players_in_lobby.push_back(name);
                    need_to_append_player = false;
                }

                char input = get_char();

                if (input == 'w') {
                    const char *send_msg = "!startgame";
                    send(client_socket, send_msg, strlen(send_msg), 0);
                    
                    run_player_join_thread.store(false);
                    run_display_room_thread.store(false);

                    run_game();
                    break;
                } else if (input == 's') {
                    const char *close_msg = "!abort";
                    send(client_socket, close_msg, strlen(close_msg), 0);

                    std::cout << "\n\nAborted the lobby!" << std::endl;
                    sleep_for(2000);

                    run_player_join_thread.store(false);
                    run_display_room_thread.store(false);
                    break;
                }
            }

        } else if (input == '2') {
            clear();
            std::string room_code;
            
            std::cout << "Enter your room code: ";
            std::getline(std::cin, room_code);

            if (room_code.length() != 5) {
                std::cout << "Invalid room code provided!" << std::endl;
                continue;
            } else if (not std::all_of(room_code.begin(), room_code.end(), ::isdigit)) {
                std::cout << "Room code only contains numbers!" << std::endl;
                continue;
            }

            room_code = "!join " + room_code;
            send(client_socket, room_code.c_str(), room_code.length(), 0);

            message = recv_msg();

            if (substr(message, 3) != "200") {
                std::cout << "An error occured!\n" << message;
                return;
            }

            const char *send_all_players_msg = "!all_players_in_lobby";
            send(client_socket, send_all_players_msg, strlen(send_all_players_msg), 0);

            message = recv_msg();
            message = message.substr(4);
            players_in_lobby = split_string_to_vector(message);

            redraw_room_menu.store(true);

            std::thread player_joins_thread(&Multisweeper::check_for_player_joins, this);
            player_joins_thread.detach();

            is_host.store(false);
            std::thread display_room_msg_thread(&Multisweeper::display_room_message, this, message);
            display_room_msg_thread.detach();

            while(true) {
                run_player_join_thread.store(true);
                run_display_room_thread.store(true);

                char input = get_char();

                if (room_aborted.load()) {
                    run_player_join_thread.store(false);
                    run_display_room_thread.store(false);

                    break;
                } else if (room_started.load()) {
                    run_player_join_thread.store(false);
                    run_display_room_thread.store(false);

                    run_game();
                    break;
                }

                if (input == 's') {
                    const char *close_msg = "!leave";
                    send(client_socket, close_msg, strlen(close_msg), 0);

                    std::cout << "\n\nLeft the lobby!" << std::endl;
                    sleep_for(2000);

                    run_player_join_thread.store(false);
                    run_display_room_thread.store(false);
                    break;
                }
            }
            
        } else if (input == '3'){
            const char* msg = "!close";
            send(client_socket, msg, strlen(msg), 0);

            break;
        }
    }
}