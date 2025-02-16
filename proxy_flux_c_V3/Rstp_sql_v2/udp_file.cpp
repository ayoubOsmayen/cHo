#include <winsock2.h>
#include <iostream>
#include <fstream>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 8192
#define SERVER_PORT 12345

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

    // Create the UDP socket for discovery
    SOCKET udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    sockaddr_in udpAddr;
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(SERVER_PORT);
    udpAddr.sin_addr.s_addr = INADDR_ANY;

    bind(udpSocket, (sockaddr*)&udpAddr, sizeof(udpAddr));

    // Set the socket to listen for broadcast messages
    char optval = 1;
    setsockopt(udpSocket, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));

    std::cout << "Server listening for discovery requests..." << std::endl;

    char buffer[1024];
    sockaddr_in clientAddr;
    int clientAddrLen = sizeof(clientAddr);

    // Wait for a discovery message
    int recvLen = recvfrom(udpSocket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &clientAddrLen);
    if (recvLen > 0) {
        std::string msg(buffer, recvLen);
        if (msg == "DISCOVER_SERVER") {
            std::cout << "Discovery request received." << std::endl;

            // Send back the server IP address to the client
            std::string serverIp = "192.168.1.100";  // Replace with your actual server IP address
            sendto(udpSocket, serverIp.c_str(), serverIp.length(), 0, (sockaddr*)&clientAddr, clientAddrLen);
            std::cout << "Sent server IP: " << serverIp << std::endl;
        }
    }

    // Create TCP socket for file transfer
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 1);

    std::cout << "Server listening on port 12345..." << std::endl;

    SOCKET clientSocket = accept(serverSocket, NULL, NULL);
    std::cout << "Client connected." << std::endl;

    receiveFile(clientSocket);

    closesocket(clientSocket);
    closesocket(serverSocket);
    closesocket(udpSocket);
    WSACleanup();

    return 0;
}
