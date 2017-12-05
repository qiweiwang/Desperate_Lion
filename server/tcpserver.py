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
	filename = 'data.mp3'
	f = open(filename, 'rb')
	l = f.read(1024)
	print l
	while (l):
		connection.send(l)
		l = f.read(1024)
	f.close()
	connection.close()
	print "done!"	
            #sleep(10)
        #message = "HIJKLM"
        #print client_address
	#se=connection.sendall(message)
	#print se
