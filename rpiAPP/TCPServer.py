#!/usr/bin/env python
 
import socket
from time import sleep

TCP_IP = '0.0.0.0'
TCP_PORT = 5005
BUFFER_SIZE = 1024

nodes = []
nodeDict = dict()

O2_threshold = 600
temp_threshold = 35

class SensorNode:

	def __init__(self, conn, addr, identifier):
		self._conn = conn
		self._addr = addr
		self._identifier = identifier
		self._O2 = 500
		self._temp = 22.0
		self._flame = 0
		self._alert = 0
		self._currentCommand = "1000"
		self._lastCommand = "1000"
		
	def getO2(self):
		return self._O2

	def getTemp(self):
		return self._temp

	def getFlame(self):
		return self._flame

	def getConn(self):
		return self._conn

	def getAddr(self):
		return self._addr

	def getIdentifier(self):
		return self._identifier

	def getAlert(self):
		return self._alert

	def transmit(self):
		#if self._currentCommand != self._lastCommand:
		self._conn.send(self._currentCommand.encode())
		self._lastCommand = self._currentCommand

	def receive(self):
		data = self._conn.recv(BUFFER_SIZE).decode()     # data = "O2,temp,flame"
		splitData = data.split(',')                     # splitData = ["02", "temp", "flame"]
		self._O2 = int(splitData[0])
		self._temp = float(splitData[1])
		self._flame = int(splitData[2])

	def analyze(self):
		if (self._O2 > O2_threshold) or (self._temp > temp_threshold) or (self._flame == 1):
			self._currentCommand = "1000"       # 1 -> LED on
			self._alert = 1
		else:
			self._currentCommand = "0"       # 0 -> LED off
			self._alert = 0



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
			newNode = SensorNode(conn, addr, identifier)
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

	"""
	while True:
		print("1: Set # of Nodes\n2: Establish TCP Server\n3: Receive TCP Message\n4: Send TCP Message\n5: Close TCP Server\n")
		user_input = int(input(">>"))

		if user_input == 1:
			number_of_nodes = int(input("How many nodes are in this network? "))

		elif user_input == 2:
			s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
			s.bind((TCP_IP, TCP_PORT))

			nodes = []
			nodeDict = dict()
			while len(nodes) < number_of_nodes:
				s.listen(1)
				conn, addr = s.accept()
				identifier = addr[0].split('.')[-1]
				if identifier in nodeDict.keys():
					pass
				else:
					identifier = addr[0].split('.')[-1]
					newNode = SensorNode(conn, addr, identifier)
					nodes.append(newNode)
					nodeDict[identifier] = len(nodes)-1       # Not 100% sure if this will be useful yet, but this dict relates position in nodes array to identifier

		elif user_input == 3:
			while(1):
				for node in nodes:
					node.receive()
					node.analyze()
					print("Node Address: %s\n" % (node.getAddr()[0]))
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
	"""

 
