#include <iostream>
#include <cstring>
#include <ctime>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

const int BUF_SIZE = 1024 * 1024; // 1 MB buffer size
const int PORT = 8888; // port number to use
const char* IP_ADDR = "192.168.1.100"; // IP address of the Windows server

void* send_data(void* arg) {
    int sock = *(int*) arg;
    char buf[BUF_SIZE];
    memset(buf, 'a', BUF_SIZE); // fill buffer with 'a' characters

    int bytes_sent = 0;
    while (bytes_sent < BUF_SIZE) {
        int bytes = send(sock, buf, BUF_SIZE, 0);
        if (bytes < 0) {
            cerr << "Error sending data" << endl;
            break;
        }
        bytes_sent += bytes;
    }
    return NULL;
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        cerr << "Error creating socket" << endl;
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr)); // zero out the struct
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP_ADDR);
    server_addr.sin_port = htons(PORT);

    if (connect(sock, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0) {
        cerr << "Error connecting to server" << endl;
        close(sock);
        return 1;
    }

    int num_threads = 4; // number of threads to use
    pthread_t threads[num_threads];
    clock_t start_time = clock(); // start timer

    for (int i = 0; i < num_threads; i++) {
        if (pthread_create(&threads[i], NULL, send_data, &sock) < 0) {
            cerr << "Error creating thread" << endl;
            break;
        }
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_t end_time = clock(); // end timer
    double time_diff = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
    double bandwidth = BUF_SIZE * num_threads / time_diff / 1024 / 1024; // in MB/s

    cout << "Bandwidth: " << bandwidth << " MB/s" << endl;

    close(sock);
    return 0;
}
