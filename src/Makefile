# Nome do executável
TARGET = pcap_washer

# Compilador
CC = gcc

# Flags de compilação
CFLAGS = -Wall -O3

# Bibliotecas necessárias
LIBS = -lpcap

# Arquivos do projeto
SRCS = $(TARGET).c
OBJS = $(SRCS:.c=.o)

# Regras padrão
all: $(TARGET)

# Regra para compilar o executável
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

# Regra para gerar os arquivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza
clean:
	rm -f $(OBJS) $(TARGET)

# Regra para rodar o programa
run: $(TARGET)
	./$(TARGET)
