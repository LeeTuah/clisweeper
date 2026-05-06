# include <iostream>
# include <cstring>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>

int main(){
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);

    sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(6741);
    inet_pton(AF_INET, "192.168.1.3", &(server_address.sin_addr));

    int connect_status = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connect_status == -1){
        std::cout << "Failed to connect to the server!\n";
        return 0;
    }

    bool name_needed = true;

    while(true){
        std::string message;

        if (name_needed){
            std::cout << "Enter name: ";
            name_needed = false;
        } else std::cout << "Enter message: ";
        std::getline(std::cin, message);

        send(client_socket, message.c_str(), message.length(), 0);
        
        char buffer[1024] = {0};
        int recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
        if (recv_bytes <= 0){
            std::cout << "You got disconnected from the server!\n";
            break;
        }

        std::string message_recv = std::string(buffer, recv_bytes);
        std::cout << message_recv << std::endl;
        
        if (message_recv == "This name is already taken, try another one!") break;

        if (message == "!close") break;
    }

    close(client_socket);

    return 0;
}