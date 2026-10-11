import socket

host = socket.gethostname()
port = 12345

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind((host, port)) 

while True : 
    data, addr = server_socket.recvfrom(1024)
    print(f'Received : {data}') 
    message = input(f'Send to {addr} : ')
    server_socket.sendto(message.encode(), (host, addr[1]))
    if 'bye' in message : 
        break

server_socket.close()