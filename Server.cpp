#include <iostream>
#include <cstring>
#include <winsock2.h>
#include <pthread.h>

#pragma comment(lib, "ws2_32.lib")

using namespace std;

const int BUF_SIZE = 1024 * 1024; // 1 MB buffer size
const int PORT = 8888; // port number to use

void* handle_connection(void* arg) {
    SOCKET sock = *(SOCKET*) arg;
    char buf[BUF_SIZE];

    while (true) {
        int bytes_received = recv(sock, buf, BUF_SIZE, 0);
        if (bytes_received < 0) {
            cerr << "Error receiving data" << endl;
            break;
        }
        if (bytes_received == 0) {
            break; // connection closed by client
        }
    }
    closesocket(sock);
    return NULL;
}

int main() {
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        cerr << "Error initializing Winsock" << endl;
        return 1;
    }

    SOCKET server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // zero out the struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        cerr << "Error binding socket" << endl;
        closesocket(server_sock);
        return 1;
    }

    if (listen(server_sock, 5) == SOCKET_ERROR) {
        cerr << "Error listening on socket" << endl;
        closesocket(server_sock);
        return 1;
    }

    while (true) {
        SOCKET client_sock = accept(server_sock, NULL, NULL);
        if (client_sock == INVALID_SOCKET) {
            cerr << "Error accepting connection" << endl;
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_connection, &client_sock) < 0) {
            cerr << "Error creating thread" << endl;
            closesocket(client_sock);
            continue;
        }
    }

    closesocket(server_sock);
    WSACleanup();
    return 0;
}
