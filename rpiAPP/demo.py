#!/usr/bin/env python
 
import socket
from time import sleep

import ecmuClass

TCP_IP = '0.0.0.0'
TCP_PORT = 5005
BUFFER_SIZE = 1024

nodes = []
nodeDict = dict()

O2_threshold = 600
temp_threshold = 35

if __name__ == "__main__":

	number_of_nodes = int(input("How many nodes are in this network? "))

	# Initialize TCP Server socket
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
	s.bind((TCP_IP, TCP_PORT))

	# Three-way Handshake with each TCP Client, build node array
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

	# Loop reading sensor data, displaying sensor data, and sending commands to node.  Break loop with Ctrl-C
	try:
		while True:
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

				node.transmit()

				#sleep(1)

	except KeyboardInterrupt:
		for node in nodes:
			node.getConn().close()