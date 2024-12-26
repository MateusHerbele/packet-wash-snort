#include <stdio.h>
#include <stdlib.h>
#include <pcap.h>
#include <string.h>
#include <stdint.h>

#define CRC_POLYNOMIAL 0x04C11DB7 // Polinômio padrão da Ethernet

uint32_t calc_crc(char* data, size_t data_size){
    uint32_t crc = 0xFFFFFFFF;

    for(size_t i = 0; i < data_size; i++){
        crc ^= (uint32_t)(data[i]) << 24;

        for(int j = 0; j < 8; j++){
            if (crc & 0x80000000){
                crc = (crc << 1) ^ CRC_POLYNOMIAL;
           }else{
                crc <<= 1;
            }
        }
    }
    return ~crc;
}

void insert_crc(char* packet){
    uint32_t crc = calc_crc(&packet[14], 20); // CRC do cabeçalho IP
    packet[34] = (crc >> 24) & 0xFF;
    packet[35] = (crc >> 16) & 0xFF;
    packet[36] = (crc >> 8) & 0xFF;
    packet[37] = crc & 0xFF;
}

void package_size_corrected(char* packet){
    packet[16] = (20 >> 8) & 0xFF; // byte mais significativo
    packet[17] = 20 & 0xFF;        // byte menos significativo
}

void recalc_checksum(char* packet){
    uint32_t sum = 0;
    for(int i = 14; i < 34; i += 2){
        if (i != 24)
            sum += (packet[i] << 8) + packet[i + 1];
    }

    while (sum > 0xFFFF){
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    uint16_t checksum = ~((uint16_t)sum);
    packet[24] = (checksum >> 8) & 0xFF; // byte mais significativo
    packet[25] = checksum & 0xFF;       // byte menos significativo
}

char* packet_wash(char* packet, uint32_t packet_size){
    char* packet_washed = calloc(38, 1);
    if (packet_washed == NULL){
        perror("Falha ao alocar memória para packet_washed!");
        exit(1);
    }
    for(int i = 0; i < 34; ++i) // 14 do frame Ethernet + 20 do cabeçalho IP
        packet_washed[i] = packet[i];

    package_size_corrected(packet_washed);
    recalc_checksum(packet_washed);
    insert_crc(packet_washed);

    return packet_washed;
}

int main(int argc, char* argv[]){
    const char* input_pcap = argv[1];
    const char* output_pcap = argv[2];

    char errbuf[PCAP_ERRBUF_SIZE];
    pcap_t* pcap_handle = pcap_open_offline(input_pcap, errbuf);
    if (!pcap_handle){
        fprintf(stderr, "Erro ao abrir arquivo PCAP: %s\n", errbuf);
        return 1;
    }

    pcap_dumper_t* pcap_dumper = NULL;
    pcap_dumper = pcap_dump_open(pcap_handle, output_pcap);
    if (!pcap_dumper){
        fprintf(stderr, "Erro ao criar arquivo PCAP de saída: %s\n", pcap_geterr(pcap_handle));
        pcap_close(pcap_handle);
        return 1;
    }

    struct pcap_pkthdr* header;
    const u_char* packet_data;
    char* packet_washed = NULL;
    uint32_t packet_number = 0;

    while (pcap_next_ex(pcap_handle, &header, &packet_data) > 0){
        printf("Pacote %u\n", packet_number);
        for(int i = 14; i < header->caplen; ++i)
            printf("%02x ", packet_data[i]);
        printf("\n");

        packet_washed = packet_wash((char*)packet_data, header->caplen);
        printf("Pacote lavado: %u\n", packet_number);
        for(int i = 14; i < 38; ++i)
            printf("%02x ", (unsigned char)packet_washed[i]);
        printf("\n");

        // Atualizar o tamanho do cabeçalho do pacote lavado
        struct pcap_pkthdr new_header = *header;
        new_header.caplen = 38;
        new_header.len = 38;

        pcap_dump((u_char*)pcap_dumper, &new_header, (u_char*)packet_washed);

        free(packet_washed);
        packet_number++;
    }

    pcap_dump_close(pcap_dumper);
    pcap_close(pcap_handle);

    printf("Processamento concluído! Pacotes salvos em '%s'\n", output_pcap);
    return 0;
}
