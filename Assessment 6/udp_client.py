import socket

host = socket.gethostname()
port = 12345

client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 

while True : 
    message = input('Send to Server : ') 
    client_socket.sendto(message.encode(), (host, port)) 
    if 'bye' in message : 
        break 
    data, addr = client_socket.recvfrom(1024) 
    print(f'Server : {data}') 