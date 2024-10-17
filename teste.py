import socket

sck = socket.socket(socket.AF_INET, socket.SOCK_RAW, socket.IPPROTO_ICMP)
sck.bind(("interface-name", 0)) 
sck.setsockopt(socket.IPPROTO_IP, socket.IP_HDRINCL, 1) # Include IP headers
sck.ioctl(socket.SIO_RCVALL, socket.RCVALL_ON) # Promiscuous mode

try:
    while True:
        print("Package " + sck.recvfrom(65565))

except KeyboardInterrupt:
    print("Terminating process...")
    sck.ioctl(socket.SIO_RCVALL, socket.RCVALL_OFF)
    socket.close()