#Python file to test the socket connection

import socket
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(("127.0.0.1", 8080))

message = input("Enter your message: ")

client_socket.sendall(message[:256].encode('utf-8'))
client_socket.close()