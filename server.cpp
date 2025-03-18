#include <iostream>
#include <string>
#include <winsock2.h>
#include <cstring> // Para strcpy

#pragma comment(lib, "Ws2_32.lib") // Vincula a biblioteca Winsock automaticamente

#define PORT 5000
#define WINDOW_SIZE 10
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

    struct sockaddr_in serv_addr, cli_addr;
    int addr_len = sizeof(cli_addr);
    Packet packet, ack_packet;
    char buffer[PACKET_SIZE];
    int expected_seq = 0;

    // Configurar endereço do servidor
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

    while (true) {
        int bytes_received = recvfrom(sock, buffer, PACKET_SIZE, 0, (struct sockaddr*)&cli_addr, &addr_len);
        if (bytes_received > 0) {
            packet = *(Packet*)buffer;
            std::cout << "Recebido pacote #" << packet.seq_num << ": " << packet.data << std::endl;

            if (packet.seq_num == expected_seq) {
                expected_seq++;
            }

            ack_packet.seq_num = 0;
            ack_packet.ack_num = expected_seq - 1;
            ack_packet.window_size = WINDOW_SIZE;
            strcpy(ack_packet.data, "ACK");
            sendto(sock, (char*)&ack_packet, PACKET_SIZE, 0, (struct sockaddr*)&cli_addr, addr_len);
            std::cout << "Enviado ACK #" << ack_packet.ack_num << std::endl;
        }
    }

    closesocket(sock);
    WSACleanup();
    return 0;
}