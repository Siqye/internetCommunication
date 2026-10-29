#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#include "config.h"

void receive_messages(SOCKET server_socket) {
    char recvbuf[DEFAULT_BUFFER_SIZE];
    int iResult;

    while (true) {
        iResult = recv(server_socket, recvbuf, DEFAULT_BUFFER_SIZE, 0);
        if (iResult > 0) {
            std::string message(recvbuf, iResult);
            std::cout << "\nServer: " << message << "\nYou: ";
            std::cout.flush();
        }
        else if (iResult == 0) {
            std::cout << "\nConnection lost.\n";
            break;
        }
        else {
            std::cerr << WSAGetLastError() << "\n";
            break;
        }
    }
}

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        std::cerr << iResult << "\n";
        return 1;
    }

    SOCKET connect_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (connect_socket == INVALID_SOCKET) {
        std::cerr << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    std::string server_ip;
    std::cout << "Enter server IP: ";
    std::getline(std::cin, server_ip);
    if (server_ip.empty()) server_ip = "localhost";

    sockaddr_in client_addr;
    client_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip.c_str(), &client_addr.sin_addr);
    client_addr.sin_port = htons(DEFAULT_PORT);

    iResult = connect(connect_socket, (SOCKADDR*)&client_addr, sizeof(client_addr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "Server connection error: " << WSAGetLastError() << "\n";
        closesocket(connect_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected.\n";

    std::thread receiver(receive_messages, connect_socket);
    receiver.detach();

    std::string input_message;
    while (true) {

        std::cout << "You: ";
        std::getline(std::cin, input_message);
        if (input_message.empty()) continue;

        iResult = send(connect_socket, input_message.c_str(), (int)input_message.length(), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Error: " << WSAGetLastError() << "\n";
            break;
        }
    }

    closesocket(connect_socket);
    WSACleanup();
    return 0;
}