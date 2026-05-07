# include <iostream>
# include <cstring>
# include <string>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>

# include "minesweeper.cpp"

class Multisweeper : public Minesweeper{
protected:
    int client_socket;
    sockaddr_in server_address;
    std::string name;

    void recieve_board_from_host();
public:
    Multisweeper(int difficulty) : Minesweeper(difficulty){
        client_socket = socket(AF_INET, SOCK_STREAM, 0);

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(6741);
        inet_pton(AF_INET, "192.168.1.3", &(server_address.sin_addr));
    }

    void run();
};

void Multisweeper::run(){
    std::string message;
    char buffer[1024] = {0};

    int connect_status = connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address));
    if (connect_status == -1){
        std::cout << "Failed to connect to the server!\n";
        return;
    }

    std::cout << "Enter your name: ";
    std::getline(std::cin, name);

    send(client_socket, name.c_str(), name.length(), 0);

    int recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
    if (recv_bytes <= 0){
        std::cout << "Connection with the server closed down!\n";
        return;
    }

    message = std::string(buffer, recv_bytes);

    if (substr(message, 3) != "200"){
        std::cout << "An error occured!\n" << message;
        return;
    }

    std::cout << "Choose one of the following: \n";
    std::cout << "1. Host Game \n2. Join Game \n3. Exit";

    char input = get_char();

    if (input == '1'){

    } else if (input == '2'){

    } else if (input == '3'){
        // send !close
    }
}

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