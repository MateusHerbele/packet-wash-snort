#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>
#include <sys/ioctl.h>

#define CRC_POLYNOMIAL 0x04C11DB7 // Polinômio padrão do Ethernet

int createSocket(char* interface) {
    int raw_socket;
    struct sockaddr_ll socket_address;
    struct ifreq ifr;

    if ((raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0) {
        perror("Erro ao criar o socket");
        exit(1);
    }

    strncpy(ifr.ifr_name, interface, IFNAMSIZ-1);

    if(ioctl(raw_socket, SIOCGIFINDEX, &ifr) == -1){
        perror("Erro ao obter o índice da interface");
        close(raw_socket);
        return -1;
    }

    int ifindex = ifr.ifr_ifindex;
    printf("Índice da interface: %d \n", ifindex);

    memset(&socket_address, 0, sizeof(struct sockaddr_ll));
    socket_address.sll_family = AF_PACKET;
    socket_address.sll_protocol = htons(ETH_P_ALL);
    socket_address.sll_ifindex = ifindex;

    if(bind((raw_socket), (struct sockaddr*)&socket_address, sizeof(socket_address)) == -1){
        perror("Erro ao associar o socket à interface");
        close(raw_socket);
        return -1;
    }

    return raw_socket;
}

uint32_t calc_crc(char* data, size_t data_size){
    uint32_t crc = 0xFFFFFFFF;

    for (size_t i = 0; i < data_size; i++) {
        crc ^= (uint32_t)(data[i]) << 24;

        for (int j = 0; j < 8; j++) {
            if (crc & 0x80000000) {
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
            } else {
                crc <<= 1;
            }
        }
    }

    return ~crc;
}

uint32_t insert_crc(char* packet){
    uint32_t crc = calc_crc(&packet[14], 20);
    packet[34] = (crc >> 24) & 0xFF;
    packet[35] = (crc >> 16) & 0xFF;
    packet[36] = (crc >> 8) & 0xFF;
    packet[37] = crc & 0xFF;
}

void package_size_corrected(char* packet){
    packet[16] = (20 >> 8) & 0xFF; // byte mais significativo
    packet[17] = 20 & 0xFF;        // byte menos significativo
}

// 6-7 byte o campo checksum
void recalc_checksum(char* packet){
    u_int32_t sum = 0;
    for(int i = 14; i < 34; i += 2){
        if(i != 24)
            sum += packet[i] + packet[i+1];
    }

    if(sum > 0xFFFF) // se houver overflow
        sum = (sum & 0xFFFF) + 1;

    u_int16_t checksum = ~((u_int16_t)sum);
    packet[24] = (checksum >> 8) & 0xFF; // byte mais significativo
    packet[25] = checksum & 0xFF;        // byte menos significativo
}

char* packet_wash(char* packet, u_int packet_size){
    char* packet_washed = calloc(38, 1);
    if(packet_washed == NULL){
        perror("Falha ao alocar memória p/ packet_washed!");
        exit(1);
    }
    for(int i = 0; i < 34; ++i) // 14 do frama ethernet e 20 do header ip
        packet_washed[i] = packet[i];
    
    package_size_corrected(packet_washed);
    recalc_checksum(packet_washed);
    insert_crc(packet_washed);

    return packet_washed;
}


int main(){
    char* interface = "dummy0";
    char* send_interface = "dummy1";
    char* buffer = calloc(16000, 1); // MTU Ethernet
    int raw_socket = createSocket(interface);
    int buffer_size = 0;
    int bytes_sent = 0;
    u_int packet_number = 0;
    char* packet_washed = NULL;
    
    // Struct para o endereço de destino
    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_IP); // Protocolo
    saddr.sll_ifindex = if_nametoindex(send_interface); // Index da interface
    
    
    while(1){
        buffer_size = recv(raw_socket, buffer, 16000, 0);
        printf("Package %u \n", packet_number);
        for(int i = 14; i < buffer_size; ++i)
            printf("%02x ", (unsigned char)buffer[i]);
        printf("\n");

        packet_washed = packet_wash(buffer, buffer_size);
        printf("Package washed: %u \n", packet_number);
        for(int i = 14; i < 38; ++i)
            printf("%02x ", (unsigned char)packet_washed[i]);
        printf("\n");

        bytes_sent = sendto(raw_socket, packet_washed, 38, 0, (struct sockaddr*)&saddr, sizeof(saddr));  
        if(bytes_sent == -1){
            perror("Erro ao enviar dados!\n");
            exit(1);
        }
        packet_number++;
    }

    close(raw_socket);

    return 0;
}
