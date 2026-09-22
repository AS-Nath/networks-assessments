import socket

host = '127.0.0.1'
port = 12345

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
client_socket.connect((host, port)) 

message = 'Hello, server!' 
client_socket.send(message.encode()) 

response = client_socket.recv(1024).decode() 
print(f'Response From Server : {response}') 

client_socket.close() 