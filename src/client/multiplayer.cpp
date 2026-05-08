# include <iostream>
# include <cstring>
# include <string>
# include <netinet/in.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <unistd.h>
# include <vector>

# include "minesweeper.cpp"

class Multisweeper : public Minesweeper{
protected:
    int client_socket;
    sockaddr_in server_address;
    std::string name;

    std::vector<std::string> players_in_lobby;

    void recieve_board_from_host();
public:
    Multisweeper(int difficulty) : Minesweeper(difficulty){
        client_socket = socket(AF_INET, SOCK_STREAM, 0);

        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(6741);
        inet_pton(AF_INET, "192.168.1.4", &(server_address.sin_addr));
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
    } message = std::string(buffer, recv_bytes);

    if (substr(message, 3) != "200"){
        std::cout << "An error occured!\n" << message;
        return;
    }

    while(true){
        clear();

        std::cout << "Choose one of the following: \n";
        std::cout << "1. Host Game \n2. Join Game \n3. Exit\n>> ";

        char input = get_char();

        if (input == '1'){
            const char* msg = "!host";
            send(client_socket, msg, strlen(msg), 0);

            int recv_bytes = recv(client_socket, buffer, sizeof(buffer), 0);
            if (recv_bytes <= 0){
                std::cout << "Connection with the server is closed down!\n";
                return;
            } message = std::string(buffer, recv_bytes);

            if (substr(message, 3) != "200"){
                std::cout << "An error occured!\n" << message;
                return;
            }
            message = message.substr(4);

            while(true) {
                clear();

                std::cout << "Successfully generated a room!\n" << message << std::endl;
                std::cout << "\nPress \'W\' to start the match.\nPress \'S\' to abort the lobby." << std::endl;
                std::cout << "Players in lobby:\n";

                for (auto iter : players_in_lobby) 
                    std::cout << &(iter) << std::endl;

                char input = get_char();

                if (input == 'w') {

                } else if (input == 's') {
                    const char *close_msg = "!abort";
                    send(client_socket, close_msg, strlen(close_msg), 0);

                    std::cout << "\n\nAborted the lobby!" << std::endl;
                    sleep_for(2000);
                    break;
                }
            }

        } else if (input == '2'){
            clear();
            std::string room_code;
            std::getline(std::cin, room_code);

            if (room_code.length() != 5) {
                std::cout << "Invalid room code provided!" << std::endl;
                continue;
            }

            room_code = "!join " + room_code;
            send(client_socket, room_code.c_str(), room_code.length(), 0); // TODO: complete join
            
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