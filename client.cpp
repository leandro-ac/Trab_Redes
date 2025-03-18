#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <fstream>
#include <winsock2.h>

#pragma comment(lib, "Ws2_32.lib") // Vincula a biblioteca Winsock

#define PORT 5000
#define MAX_PACKETS 10000
#define PACKET_SIZE sizeof(Packet)
#define TIMEOUT_SEC 1 // Timeout em segundos

struct Packet {
    int seq_num;      // Número de sequência
    int ack_num;      // Número do último ACK
    int window_size;  // Tamanho da janela do destinatário
    char data[1000];  // Payload
};

int main() {
    // Inicializar Winsock
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

    // Configurar timeout
    int timeout = TIMEOUT_SEC * 1000; // Em milissegundos
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr); // Substituí socklen_t por int
    Packet packet, ack_packet;
    char buffer[PACKET_SIZE];
    int sent_packets = 0;
    int last_ack = -1;
    int cwnd = 1;         // Janela de congestionamento
    int ssthresh = 16;    // Limiar inicial
    int dup_ack_count = 0;
    std::vector<double> throughput_with_congestion;

    // Configurar endereço do servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::string data = "Dados de teste para o trabalho final";

    // Registrar início do tempo
    auto start_time = std::chrono::high_resolution_clock::now();

    while (sent_packets < MAX_PACKETS) {
        // Determinar tamanho da janela
        int window = (last_ack == -1) ? cwnd : std::min(cwnd, ack_packet.window_size);

        for (int i = 0; i < window && sent_packets < MAX_PACKETS; i++) {
            packet.seq_num = sent_packets;
            packet.ack_num = 0;
            packet.window_size = 0;
            strcpy(packet.data, data.c_str());
            sendto(sock, (char*)&packet, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, addr_len);
            std::cout << "Enviado pacote #" << sent_packets << std::endl;
            sent_packets++;
        }

        int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, nullptr, nullptr);
        if (bytes_received > 0) {
            memcpy(&ack_packet, buffer, PACKET_SIZE); // Copiar buffer para struct
            if (ack_packet.ack_num == last_ack) {
                dup_ack_count++;
                if (dup_ack_count == 3) {
                    ssthresh = std::max(cwnd / 2, 1);
                    cwnd = ssthresh;
                    dup_ack_count = 0;
                    std::cout << "3 ACKs duplicados, cwnd = " << cwnd << ", ssthresh = " << ssthresh << std::endl;
                }
            } else if (ack_packet.ack_num > last_ack) {
                last_ack = ack_packet.ack_num;
                dup_ack_count = 0;
                if (cwnd < ssthresh) cwnd++; // Slow Start
                else cwnd += 1;              // Congestion Avoidance
                std::cout << "Recebido ACK #" << last_ack << ", cwnd = " << cwnd << std::endl;
            }

            // Calcular vazão
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            throughput_with_congestion.push_back((last_ack + 1) / elapsed);
        } else { // Timeout
            ssthresh = std::max(cwnd / 2, 1);
            cwnd = 1;
            dup_ack_count = 0;
            std::cout << "Timeout, cwnd = " << cwnd << ", ssthresh = " << ssthresh << std::endl;
        }
    }

    // Salvar dados de vazão com controle de congestionamento
    std::ofstream file("throughput_with_congestion.csv");
    file << "Tempo,Vazao\n";
    for (size_t i = 0; i < throughput_with_congestion.size(); i++)
        file << i << "," << throughput_with_congestion[i] << "\n";
    file.close();

    // Teste sem controle de congestionamento
sent_packets = 0;
last_ack = -1;
std::vector<double> throughput_without_congestion;
start_time = std::chrono::high_resolution_clock::now();

while (sent_packets < MAX_PACKETS) {
    packet.seq_num = sent_packets;
    strcpy(packet.data, data.c_str());
    sendto(sock, (char*)&packet, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, addr_len);
    std::cout << "Enviado pacote #" << sent_packets << " (sem controle)" << std::endl;
    sent_packets++;

    int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, nullptr, nullptr);
    if (bytes_received > 0) {
        memcpy(&ack_packet, buffer, PACKET_SIZE);
        if (ack_packet.ack_num > last_ack) {
            last_ack = ack_packet.ack_num;
            auto now = std::chrono::high_resolution_clock::now();
            double elapsed = std::chrono::duration<double>(now - start_time).count();
            throughput_without_congestion.push_back((last_ack + 1) / elapsed);
        }
    }
}

    // Salvar dados de vazão sem controle de congestionamento
    file.open("throughput_without_congestion.csv");
    file << "Tempo,Vazao\n";
    for (size_t i = 0; i < throughput_without_congestion.size(); i++)
        file << i << "," << throughput_without_congestion[i] << "\n";
    file.close();

    closesocket(sock);
    WSACleanup();
    std::cout << "Envio concluido!" << std::endl;
    return 0;
}