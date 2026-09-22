import socket

host_addr = '127.0.0.1'
port = 12345

server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 

server_socket.bind((host_addr, port)) 
server_socket.listen(1) 

print(f'Server Socket listening on {host_addr}, {port}')

client_socket, addr = server_socket.accept() 
print(f'Connection Established from {addr}')

data = client_socket.recv(1024).decode() 
print(f'Data Received from Client : {data}') 

response = 'Message Received Successfully :)' 
client_socket.send(response.encode()) 

client_socket.close()
server_socket.close() 