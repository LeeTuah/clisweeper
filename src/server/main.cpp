# include <iostream>
# include <cstring>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <thread>
# include <map>
# include <vector>
# include <algorithm>

# include "utils.cpp"

struct Client{
    int socket;
    std::string name;
    bool has_room;
    std::string roomate;
    int room_code;
};

int server_socket;
std::map<std::string, Client> client_map;
std::vector<int> all_rooms;

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
                const char *res = "403|This name is already taken, try another one!";
                send(client.socket, res, strlen(res), 0);
                std::cout << "Client kicked out for duplicate username.\n";
                break;
            }

            client.name = message;
            client.has_room = false;
            client.room_code = 0;
            client.roomate = "";
            client_map.insert({client.name, client});

            const char *res = "200|Successfully entered you to the server!";
            send(client.socket, res, strlen(res), 0);
            std::cout << client.name << " joined the server.\n" << std::flush;

            continue;
        } else if (message == "!close") {
            const char *res = "200|Closed your connection successfully.";
            send(client.socket, res, strlen(res), 0);

            if (client.name != "") std::cout << client.name << " requested to close the connection!\n";
            else std::cout << "Client requested to close the connection!\n";

            break;
        } else if (message == "!host") {
            if (client.has_room) {
                const char *res = "403|You already have a room!";
                send(client.socket, res, strlen(res), 0);
                continue;
            }

            int r_code = random_number(10000, 99999);

            while (std::find(all_rooms.begin(), all_rooms.end(), r_code) != all_rooms.end()) // if duplicate room id generated
                r_code = random_number(10000, 99999);

            client.has_room = true;
            client.room_code = r_code;

            client_map.at(client.name) = client;
            all_rooms.push_back(r_code);

            std::string message = "200|Your room ID is " + std::to_string(r_code);
            send(client.socket, message.c_str(), message.length(), 0);
            std::cout << client.name << " generated a new room with ID " << r_code << std::endl;
        } else if (message == "!abort") {
            if (not client.has_room){
                const char *res = "403|You do not have a room!";
                send(client.socket, res, strlen(res), 0);
                continue;
            }

            all_rooms.erase(std::remove(all_rooms.begin(), all_rooms.end(), client.room_code), all_rooms.end());
            std::cout << "Destroyed room of " << client.name << " with ID " << client.room_code << "." << std::endl; 

            client.has_room = false;
            client.roomate = ""; // FIXME: kick roomate from room if any
            client.room_code = 0;
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