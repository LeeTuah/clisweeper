# include <iostream>
# include <cstring>
# include <netinet/in.h>
# include <sys/socket.h>
# include <unistd.h>
# include <thread>
# include <map>
# include <vector>

struct Client{
    int socket;
    std::string name;
    bool has_room;
    std::string roomate;
};

int server_socket;
std::map<std::string, Client> client_map;

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
            
        }
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