#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 8192

void initWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed." << std::endl;
        exit(1);
    }
}

void receiveFile(SOCKET clientSocket) {
    char buffer[BUFFER_SIZE];
    int bytesRead;
    std::ofstream file("received_file", std::ios::binary);

    while ((bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0)) > 0) {
        file.write(buffer, bytesRead);
    }

    std::cout << "File received successfully." << std::endl;
    file.close();
}

int main() {
    initWinsock();

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(12345);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 1);

    std::cout << "Server listening on port 12345..." << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    std::cout << "Client connected." << std::endl;

    receiveFile(clientSocket);

    closesocket(clientSocket);
    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
