#include <iostream>
#include <string>
#include <winsock2.h>
#include <cstring> // Para strcpy

#pragma comment(lib, "Ws2_32.lib") // Vincula a biblioteca Winsock automaticamente

#define PORT 5000
#define MAX_PACKETS 10000
#define PACKET_SIZE sizeof(Packet)

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

    struct sockaddr_in serv_addr;
    int addr_len = sizeof(serv_addr);
    Packet packet, ack_packet;
    char buffer[PACKET_SIZE];
    int sent_packets = 0;
    int last_ack = -1;

    // Configurar endereço do servidor
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    std::string data = "Dados de teste";

    while (sent_packets < MAX_PACKETS) {
        // Enviar pacote
        packet.seq_num = sent_packets;
        packet.ack_num = 0;
        packet.window_size = 0;
        strcpy(packet.data, data.c_str()); // Substituído strcpy_s por strcpy
        sendto(sock, (char*)&packet, PACKET_SIZE, 0, (struct sockaddr*)&serv_addr, addr_len);
        std::cout << "Enviado pacote #" << sent_packets << std::endl;
        sent_packets++;

        // Receber ACK (sem timeout complexo)
        int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, nullptr, nullptr);
        if (bytes_received > 0) {
            ack_packet = *(Packet*)buffer;
            if (ack_packet.ack_num > last_ack) {
                last_ack = ack_packet.ack_num;
                std::cout << "Recebido ACK #" << last_ack << std::endl;
            }
        }
    }

    closesocket(sock);
    WSACleanup();
    std::cout << "Envio concluído!" << std::endl;
    return 0;
}