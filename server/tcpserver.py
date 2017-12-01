import socket
import sys

sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
print "socket created"
server_address = ("192.168.0.211", 2077)

sock.bind(server_address)
print "bind success"
sock.listen(1)

message = "ABCDEFG"

while True:
	connection, client_address = sock.accept()
	while True:
            print client_address
	    se=connection.sendall(message)
	    print se
            #sleep(10)
        #message = "HIJKLM"
        #print client_address
	#se=connection.sendall(message)
	#print se
