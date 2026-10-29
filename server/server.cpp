#include <iostream>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <string>

#include "config.h"

void receive_messages(SOCKET client_socket) {
    char recvbuf[DEFAULT_BUFFER_SIZE];
    int iResult;

    while (true) {
        iResult = recv(client_socket, recvbuf, DEFAULT_BUFFER_SIZE, 0);
        if (iResult > 0) {
            std::string message(recvbuf, iResult);
            std::cout << "\nClient: " << message << "\nYou(server): ";
            std::cout.flush();
        }
        else if (iResult == 0) {
            std::cout << "\nClient disconnected.\n";
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

    SOCKET listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_socket == INVALID_SOCKET) {
        std::cerr << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(DEFAULT_PORT);

    iResult = bind(listen_socket, (SOCKADDR*)&server_addr, sizeof(server_addr));
    if (iResult == SOCKET_ERROR) {
        std::cerr << "bind: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    iResult = listen(listen_socket, SOMAXCONN);
    if (iResult == SOCKET_ERROR) {
        std::cerr << "socket erro: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server started. waiting for client...\n";

    SOCKET client_socket = accept(listen_socket, NULL, NULL);
    if (client_socket == INVALID_SOCKET) {
        std::cerr << "accept: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Connected\n";
    closesocket(listen_socket);

    std::thread receiver(receive_messages, client_socket);
    receiver.detach();

    std::string input_message;
    while (true) {
        std::cout << "You(server): ";
        std::getline(std::cin, input_message);
        if (input_message.empty()) continue;

        iResult = send(client_socket, input_message.c_str(), (int)input_message.length(), 0);
        if (iResult == SOCKET_ERROR) {
            std::cerr << "Error: " << WSAGetLastError() << "\n";
            break;
        }
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}