# include <iostream>
# include <cstring>
# include <string>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <thread>
# include <map>
# include <vector>
# include <algorithm>
# include <cctype>

# include "utils.cpp"

/*
My concept for Multisweeper:

Design inspired from the battleships board game 

1. Players get a set time to place bombs in their own boards.
2. After both ready, their boards are swapped.
3. Now, both have to guess where the mines are.
4. First one to die loses, or first one to complete their area wins.

Flaws:

1. Player can make an impossible board, which does not have any solutions
Solutions: 1. Make an algorithm to check for impossible boards (more complicated, but rewarding)
           2. Make it so that players do not lose the game instantly when this happens, they just get a penalty (less complicated, but breaks my flow)
*/

struct Client{
    int socket;
    std::string name;

    bool has_room;
    bool is_host;
    int room_code;

    bool playing_ingame;
    bool is_won;
};

int PLAYER_CAP = 2;
int total_mul_bombs = 14;

int server_socket;
std::map<std::string, Client*> client_map;
std::map<int, std::vector<Client*>> all_rooms;

void send_response(int client_socket, std::string message){
    send(client_socket, message.c_str(), message.length(), 0);
}

bool room_exists(int room_code){
    for (auto const &[key, val] : all_rooms)
        if (key == room_code) return true;

    return false;
}

void handle_connections(Client client){
    bool name_needed = true;
    Client *opponent;

    bool collect_bombs = true;
    int bombs_left_to_collect = total_mul_bombs; // TODO reset them at the end of the game

    while(true){
        char buffer[1024] = {0};
        std::string message;
        int recv_bytes = recv(client.socket, buffer, sizeof(buffer), 0);

        if (recv_bytes == 0){
            if (client.name != "") std::cout << client.name << " has disconnected!\n";
            else std::cout << "Client has disconnected!\n";
            break;
        } else if (recv_bytes < 0) {
            std::cout << "An error occured with the sockets!\n";
            break;
        }
        
        message = std::string(buffer, recv_bytes);

        if (name_needed){
            name_needed = false;

            if (client_map.find(message) != client_map.end()){
                send_response(client.socket, "403|This name is already taken, try another one!");
                std::cout << "Client kicked out for duplicate username.\n";
                break;
            }

            client.name = message;
            client.has_room = false;
            client.is_host = false;
            client.room_code = 0;
            client.playing_ingame = false;
            client.is_won = false;
            client_map.insert({client.name, &client});

            send_response(client.socket, "200|Successfully entered you to the server!");
            std::cout << client.name << " joined the server.\n" << std::flush;

            continue;
        } else if (client.playing_ingame) {
            if (collect_bombs) {
                send_response(opponent->socket, message);

                if (bombs_left_to_collect == 0) collect_bombs = false;
                continue;
            }


        } else if (message == "!close") {
            send_response(client.socket, "200|Closed your connection successfully.");

            if (client.name != "") std::cout << client.name << " requested to close the connection!\n";
            else std::cout << "Client requested to close the connection!\n";

            break;
        } else if (message == "!host") {
            if (client.has_room) {
                send_response(client.socket, "403|You already have a room!");
                continue;
            }

            int r_code = random_number(10000, 99999);

            while (room_exists(r_code)) // if duplicate room id generated
                r_code = random_number(10000, 99999);

            client.has_room = true;
            client.room_code = r_code;

            all_rooms.insert({r_code, {&client}});

            send_response(client.socket, "200|Your room ID is " + std::to_string(r_code));
            std::cout << client.name << " generated a new room with ID " << r_code << std::endl;
        } else if (message == "!abort") {
            if (not client.has_room){
                send_response(client.socket, "403|You do not have a room!");
                continue;
            }

            for (auto peer : all_rooms[client.room_code]) {
                if (peer->socket == client.socket) continue;
                send_response(peer->socket, "205|The host has aborted his room!");

                peer->has_room = false;
                peer->room_code = false;
            }

            all_rooms.erase(client.room_code);
            send_response(client.socket, "200|Successfully aborted your room!");
            std::cout << "Destroyed room of " << client.name << " with ID " << client.room_code << "." << std::endl; 

            client.has_room = false;
            client.is_host = false;
            client.room_code = 0;
        } else if (substr(message, 5) == "!join") {
            if (client.has_room or client.playing_ingame){
                send_response(client.socket, "403|You are already in a lobby!");
                continue;
            }

            if (message.length() != 1 + 4 + 1 + 5) {
                send_response(client.socket, "403|Invalid join message provided!");
                continue;
            }

            std::string room_code = message.substr(6);
            if (not std::all_of(room_code.begin(), room_code.end(), ::isdigit)) {
                send_response(client.socket, "403|Invalid room code provided!");
                continue;
            }

            int r_code = std::stoi(room_code);
            if (not room_exists(r_code)) {
                send_response(client.socket, "403|The given room does not exist!");
                continue;
            }

            if (all_rooms[r_code].size() == PLAYER_CAP) {
                send_response(client.socket, "403|The lobby is currently full!");
                continue;
            }

            for (auto _client : all_rooms[r_code])
                send_response(_client->socket, "201|" + client.name);

            client.has_room = true;
            client.is_host = false;
            client.room_code = r_code;

            all_rooms[r_code].push_back(&client);
            send_response(client.socket, "200|Successfully joined the room!");
            std::cout << client.name << " joined the room with ID " << r_code << std::endl;
        } else if (message == "!all_players_in_lobby"){
            std::string players_list = "";

            for (auto client : all_rooms[client.room_code])
                players_list += client->name + ",";

            send_response(client.socket, "200|" + players_list);
        } else if (message == "!leave") {
            if (not client.has_room) {
                send_response(client.socket, "403|You are not in a room.");
                continue;
            }

            if (client.is_host) {
                send_response(client.socket, "403|Hosts cannot leave their rooms. Abort to proceed.");
                continue;
            }

            std::vector<Client*> *vec = &(all_rooms[client.room_code]);
            auto iter = std::find(vec->begin(), vec->end(), &client);
            vec->erase(iter);

            for (auto _client : all_rooms[client.room_code]) 
                send_response(_client->socket, "202|" + client.name);

            client.has_room = false;
            client.is_host = false;

            send_response(client.socket, "200|Removed you from the lobby.");
            std::cout << client.name << " left the room with ID " << client.room_code << std::endl;
            client.room_code = 0;
        } else if (message == "!startgame") {
            if (not client.has_room) {
                send_response(client.socket, "403|You are not in a room!");
                continue;
            }

            if (not client.is_host) {
                send_response(client.socket, "403|You are not the host!");
                continue;
            }

            for (auto _client : all_rooms[client.room_code]) {
                _client->playing_ingame = true;
                send_response(_client->socket, "670|The game has started!");

                if (_client->socket != client.socket) opponent = _client;
            }
        }
    }

    if (client.has_room) { 
        all_rooms.erase(client.room_code);
        std::cout << "Destroyed room of " << client.name << " with ID " << client.room_code << "." << std::endl; 
    }

    client_map.erase(client.name);
    close(client.socket);
}

int main(){
    clear();

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1){
        std::cout << "Failed to initialize the socket!";
        return 0;
    }

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(6741);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address));

    listen(server_socket, 7);
    std::cout << "The server is currently listening..." << std::endl;

    while(true){
        Client client;
        client.socket = accept(server_socket, nullptr, nullptr);
        
        std::thread client_thread(handle_connections, client);
        client_thread.detach();
    }
    
    close(server_socket);

    return 0;
}