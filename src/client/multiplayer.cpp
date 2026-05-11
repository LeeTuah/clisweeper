# include <iostream>
# include <cstring>
# include <string>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <vector>
# include <cctype>
# include <atomic>

# include "minesweeper.cpp"

class Multisweeper : public Minesweeper{
protected:
    int client_socket;
    sockaddr_in server_address;
    std::string name;

    std::atomic<bool> run_player_join_thread;
    std::atomic<bool> run_display_room_thread;
    std::atomic<bool> room_aborted;
    std::vector<std::string> players_in_lobby;
    bool need_to_append_player;

    std::atomic<bool> is_host;
    std::atomic<bool> redraw_room_menu;

    std::string recv_msg();
    void check_for_player_joins();
    void display_room_message(std::string message = "");
    void recieve_board_from_host();
    void run_game();
public:
    Multisweeper(int difficulty) : Minesweeper(difficulty){
        client_socket = socket(AF_INET, SOCK_STREAM, 0);

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(6741);
        inet_pton(AF_INET, "192.168.1.4", &(server_address.sin_addr));

        multiplayer_gamemode = true;
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
        }
    }
}

void Multisweeper::display_room_message(std::string message /*= ""*/) {
    while (run_display_room_thread.load()) { // FIXME
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

            if (room_aborted.load()) {
                std::cout << "[!] The host has aborted the room!\n[!] Press any key to continue.....\n";
            }

            redraw_room_menu.store(false);
        }
    }
}

void Multisweeper::run_game() {
    if (placing_bombs) {
        std::cout << _YELLOW << "[!] You have x seconds to place 15 bombs in the given board!" << RESET;
    }

    display_board();
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
                    // TODO: mutex here for thread confusions
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

int main(){
    Multisweeper m(1);
    m.run();

    return 0;
}