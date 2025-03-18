# Implementação de um Protocolo Confiável sobre UDP

## Compilação e Execução

1. **Compile os arquivos**:

    g++ client.cpp -o client.exe -lws2_32
    g++ server.cpp -o server.exe -lws2_32

2. **Execute o servidor**:

    server.exe

 
3. **Execute o cliente (em outro terminal)**:

    client.exe


Os arquivos `throughput_with_congestion.csv`, `throughput_without_congestion.csv`, e `throughput_with_loss.csv` serão gerados com os dados de vazão.