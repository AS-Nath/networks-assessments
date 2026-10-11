import socket

host = socket.gethostname()
port = 12345

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
server_socket.bind((host, port)) 

server_socket.listen(1)

client_socket, addr = server_socket.accept() 
print(f'Connection Accepted at Port {addr}')

while True : 
    data = client_socket.recv(1024).decode()
    print(f'Client : {data}') 
    message = input('Send Message : ') 
    if 'bye' in message : 
        break
    client_socket.send(message.encode()) 

server_socket.close() 