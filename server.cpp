#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <fstream>
#include <winsock2.h>
#include <cstdlib>
#include <ctime>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 5000
#define WINDOW_SIZE 10
#define MAX_PACKETS 10000
#define PACKET_SIZE sizeof(Packet)
#define LOSS_RATE 0.1

struct Packet {
    int seq_num;
    int ack_num;
    int window_size;
    char data[1000];
};

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Erro ao inicializar Winsock" << std::endl;
        return -1;
    }

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Erro ao criar socket" << std::endl;
        WSACleanup();
        return -1;
    }

    struct sockaddr_in serv_addr, cli_addr;
    int addr_len = sizeof(cli_addr);
    Packet packet, ack_packet;
    char buffer[PACKET_SIZE];
    int expected_seq = 0; // Garantir início em 0
    int received_packets = 0;
    std::map<int, Packet> buffer_packets;
    std::vector<double> throughput_with_loss;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);
    if (bind(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == SOCKET_ERROR) {
        std::cerr << "Erro no bind" << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    std::cout << "Servidor rodando na porta " << PORT << "..." << std::endl;
    srand(static_cast<unsigned>(time(0)));
    auto start_time = std::chrono::high_resolution_clock::now();

    while (received_packets < MAX_PACKETS) {
        int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, (struct sockaddr*)&cli_addr, &addr_len);
        if (bytes_received <= 0) continue;

        memcpy(&packet, buffer, PACKET_SIZE);

        if ((double)rand() / RAND_MAX < LOSS_RATE) {
            std::cout << "Pacote #" << packet.seq_num << " perdido (simulado)" << std::endl;
            ack_packet.seq_num = 0;
            ack_packet.ack_num = expected_seq - 1;
            ack_packet.window_size = WINDOW_SIZE;
            strcpy(ack_packet.data, "ACK");
            sendto(sock, (char*)&ack_packet, PACKET_SIZE, 0, (struct sockaddr*)&cli_addr, addr_len);
            continue;
        }

        buffer_packets[packet.seq_num] = packet;
        std::cout << "Recebido pacote #" << packet.seq_num << std::endl;

        while (buffer_packets.find(expected_seq) != buffer_packets.end()) {
            expected_seq++;
            received_packets++;
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            if (elapsed > 0)
                throughput_with_loss.push_back(received_packets / elapsed);
        }

        ack_packet.seq_num = 0;
        ack_packet.ack_num = expected_seq - 1;
        ack_packet.window_size = WINDOW_SIZE;
        strcpy(ack_packet.data, "ACK");
        sendto(sock, (char*)&ack_packet, PACKET_SIZE, 0, (struct sockaddr*)&cli_addr, addr_len);
        std::cout << "Enviado ACK #" << ack_packet.ack_num << std::endl;
    }

    std::ofstream file("throughput_with_loss.csv");
    file << "Tempo,Vazao\n";
    for (size_t i = 0; i < throughput_with_loss.size(); i++)
        file << i << "," << throughput_with_loss[i] << "\n";
    file.close();

    closesocket(sock);
    WSACleanup();
    std::cout << "Recepcao concluida!" << std::endl;
    return 0;
}