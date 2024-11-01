#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <net/if.h>
#include <string.h>

int createSocket(char* interface) {
    int raw_socket;
    struct packet_mreq mr;

    // Cria o socket raw para pacotes IP
    if ((raw_socket = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP))) < 0) {
        perror("Erro ao criar o socket");
        exit(1);
    }

    // Configura o socket para modo promíscuo
    mr.mr_ifindex = if_nametoindex(interface);
    mr.mr_type = PACKET_MR_PROMISC;

    if (setsockopt(raw_socket, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) < 0) {
        printf("Interface bind error!");
        exit(1);
    }

    return raw_socket;
}

void package_size_corrected(char* packet){
    packet[16] = (20 >> 8) & 0xFF; // byte mais significativo
    packet[17] = 20 & 0xFF;        // byte menos significativo
}

// 6-7 byte o campo checksum
void recalc_checksum(char* packet){
    u_int32_t sum = 0;
    for(int i = 14; i < 34; i += 2){
        if(i != 20)
            sum += packet[i] + packet[i+1];
    }

    if(sum > 0xFFFF) // se houver overflow
        sum = (sum & 0xFFFF) + 1;

    u_int16_t checksum = ~((u_int16_t)sum);
    packet[20] = (checksum >> 8) & 0xFF; // byte mais significativo
    packet[21] = checksum & 0xFF;        // byte menos significativo
}

char* packet_wash(char* packet, u_int packet_size){
    char* packet_washed = calloc(34, 1);
    if(packet_washed == NULL){
        printf("Failed to alloc memory to packet_washed!");
        exit(1);
    }
    for(int i = 0; i < 34; ++i)
        packet_washed[i] = packet[i];
    
    package_size_corrected(packet_washed);
    recalc_checksum(packet_washed);

    return packet_washed;
}


int main(){
    char* interface = "dummy0";
    char* send_interface = "dummy1";
    char* buffer = calloc(1500, 1); // MTU Ethernet
    int raw_socket = createSocket(interface);
    int send_socket = createSocket(send_interface);
    int buffer_size = 0;
    int bytes_sent = 0;
    u_int packet_number = 0;
    char* packet_washed = NULL;
    
    // Struct para o endereço de destino
    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL); // Protocolo
    saddr.sll_ifindex = if_nametoindex(send_interface); // Index da interface
    
    while(1){
        buffer_size = recv(raw_socket, buffer, 1500, 0);
        printf("Package %u \n", packet_number);
        for(int i = 14; i < buffer_size; ++i)
            printf("%02x ", (unsigned char)buffer[i]);
        printf("\n");

        packet_washed = packet_wash(buffer, buffer_size);
        printf("Package washed: %u \n", packet_number);
        for(int i = 14; i < 34; ++i)
            printf("%02x ", (unsigned char)packet_washed[i]);
        printf("\n");

        bytes_sent = sendto(send_socket, packet_washed, 34, 0, (struct sockaddr*)&saddr, sizeof(saddr));        if(bytes_sent == -1){
            printf("Error sending data!\n");
            exit(1);
        }
        // exit(1);?
        packet_number++;
    }

    return 0;
}
