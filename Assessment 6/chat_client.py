import socket

host = socket.gethostname()
port = 12345

client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
client_socket.connect((host, port)) 

while True : 
    message = input('Send Message : ') 
    if 'bye' in message.lower() : 
        break 
    client_socket.send(message.encode()) 
    data = client_socket.recv(1024).decode() 
    print(f'Server : {data}') 

client_socket.close() 