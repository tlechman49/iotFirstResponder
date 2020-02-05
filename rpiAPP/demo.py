#!/usr/bin/env python

import socket
from time import sleep

import ecmuClass

TCP_IP = '0.0.0.0'
TCP_PORT = 5005
BUFFER_SIZE = 1024

# nodes = []
# nodeDict = dict()

O2_threshold = 600
temp_threshold = 35

if __name__ == "__main__":

    number_of_nodes = int(input("How many nodes are in this network? "))

    ecmuSet = ecmuClass.ecmuSet(number_of_nodes)

    # Initialize TCP Server socket
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.bind((TCP_IP, TCP_PORT))

    # Three-way Handshake with each TCP Client, build node array
    while ecmuSet.getLenNodeList() < ecmuSet.getMaxNodes():
        s.listen(1)
        conn, addr = s.accept()
        identifier = conn.recv(BUFFER_SIZE).decode()    # use a portion of the node's MAC address as an identifier
        if ecmuSet.isEcmu(identifier):
            pass
        else:
            ecmuSet.addEcmu(conn, addr, identifier)

    # Loop reading sensor data, displaying sensor data, and sending commands to node.  Break loop with Ctrl-C
    try:
        while True:
            ecmuSet.receive()
            ecmuSet.analyze()
            ecmuSet.print()
            ecmuSet.transmit()
            # sleep(1)

    except KeyboardInterrupt:
        ecmuSet.disconnect()