#!/usr/bin/env python
 
import socket
from time import sleep

import ecmuClass

TCP_IP = '0.0.0.0'
TCP_PORT = 5005
BUFFER_SIZE = 1024

nodes = []
nodeDict = dict()

if __name__ == "__main__":

	number_of_nodes = 2
	
	while True:
		print("1: Set # of Nodes\n2: Establish TCP Server\n3: Receive TCP Message\n4: Send TCP Message\n5: Close TCP Server\n")
		user_input = int(input(">>"))

		if user_input == 1:
			number_of_nodes = int(input("How many nodes are in this network? "))

		elif user_input == 2:
			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.bind((TCP_IP, TCP_PORT))

			while len(nodes) < number_of_nodes:
				s.listen(1)
				conn, addr = s.accept()
				identifier = addr[0].split('.')[-1]
				if identifier in nodeDict.keys():
					pass
				else:
					identifier = conn.recv(BUFFER_SIZE).decode()	# use a portion of the node's MAC address as an identifier
					nodeDict[identifier] = len(nodes)
					newNode = ecmuClass.ecmu(conn, addr, identifier)
					nodes.append(newNode)

		elif user_input == 3:
			for node in nodes:
				node.receive()
				node.analyze()
				print("Node: %s\n" % (node.getIdentifier()))
				print("IP Address: %s\n" % (node.getAddr()[0]))
				print("O2 Sensor Reading: %d\n" % (node.getO2()))
				print("Temperature Sensor Reading: %f\n" % (node.getTemp()))
				print("Fire Detected: %s\n" % (node.getFlame()))

				if node.getAlert() == 1:
					print("ALERT\n")

				print("\n")

		elif user_input == 4:
			for node in nodes:
				node.transmit()
 
		elif user_input == 5:
			for node in nodes:
				node.getConn().close()
			break

		else:
			print("Invalid input, please try again\n")
	

 
