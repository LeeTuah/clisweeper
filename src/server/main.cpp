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

struct Client{
    int socket;
    std::string name;
    bool has_room;
    std::vector<std::string> roomates;
    int room_code;
    bool playing_ingame;
};

int server_socket;
std::map<std::string, Client*> client_map;
std::vector<int> all_rooms;

void send_response(int client_socket, std::string message){
    send(client_socket, message.c_str(), message.length(), 0);
}

void handle_connections(Client client){
    bool name_needed = true;

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
            client.room_code = 0;
            client.roomates = {};
            client.playing_ingame = false;
            client_map.insert({client.name, &client});

            send_response(client.socket, "200|Successfully entered you to the server!");
            std::cout << client.name << " joined the server.\n" << std::flush;

            continue;
        } else if (client.playing_ingame) {
            // playing the game
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

            while (std::find(all_rooms.begin(), all_rooms.end(), r_code) != all_rooms.end()) // if duplicate room id generated
                r_code = random_number(10000, 99999);

            client.has_room = true;
            client.room_code = r_code;

            all_rooms.push_back(r_code);

            send_response(client.socket, "200|Your room ID is " + std::to_string(r_code));
            std::cout << client.name << " generated a new room with ID " << r_code << std::endl;
        } else if (message == "!abort") {
            if (not client.has_room){
                send_response(client.socket, "403|You do not have a room!");
                continue;
            }

            all_rooms.erase(std::remove(all_rooms.begin(), all_rooms.end(), client.room_code), all_rooms.end());
            std::cout << "Destroyed room of " << client.name << " with ID " << client.room_code << "." << std::endl; 

            client.has_room = false;
            client.roomates = {}; // FIXME: kick roomate from room if any
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
            if (std::find(all_rooms.begin(), all_rooms.end(), r_code) == all_rooms.end()) {
                send_response(client.socket, "403|The given room does not exist!");
                continue;
            }
        } else if (message == "!startgame") {
            client.playing_ingame = true;
            // FIXME: set playing_ingame for other clients too
        }
    }

    if (client.has_room) { 
        // TODO: kick roomate if present

        all_rooms.erase(std::remove(all_rooms.begin(), all_rooms.end(), client.room_code), all_rooms.end());
        std::cout << "Destroyed room of " << client.name << " with ID " << client.room_code << "." << std::endl; 
    }

    client_map.erase(client.name);
    close(client.socket);
}

int main(){
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