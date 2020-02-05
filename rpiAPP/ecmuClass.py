# defining some thresholds
O2_threshold = 600
temp_threshold = 35

# defining output types to numeric values
ONBOARD_LED = 0
LED_STRIP = 1

# class to store and work with individual outputs


class ecmuOutput:
    def __init__(self, outputType, pin):
        self._outputType = outputType
        self._pin = pin
        self._curMsg = 0
        self._lastMsg = 0


# class used to store and work with individual nodes
class ecmu:
    def __init__(self, conn, addr, identifier):
        self._conn = conn
        self._addr = addr
        self._identifier = identifier
        self._O2 = 500
        self._temp = 22.0
        self._flame = 0
        self._alert = 0
        self._currentCommand = "0.13.1000"
        self._lastCommand = "0.13.1000"
        self._outputList = []

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

    # transmit output messages to node
    def transmit(self):
        if self._currentCommand != self._lastCommand:
            self._conn.send(self._currentCommand.encode())
            self._lastCommand = self._currentCommand

    # receive sensor data from node
    def receive(self):
        data = self._conn.recv(1024).decode()     # data = "O2,temp,flame"
        # splitData = ["O2", "temp", "flame"]
        splitData = data.split(',')
        self._O2 = int(splitData[0])
        self._temp = float(splitData[1])
        self._flame = int(splitData[2])

    # analyze node data to perform automated operations
    def analyze(self):
        if (self._O2 > O2_threshold) or (self._temp > temp_threshold) or (self._flame == 1):
            self._currentCommand = "0.13.1000"       # 1 -> LED on
            self._alert = 1
        else:
            self._currentCommand = "0.13.0"       # 0 -> LED off
            self._alert = 0

# class used to store and work with the set of nodes


class ecmuSet:
    def __init__(self, maxNodes):
        self.maxNodes = maxNodes
        self.nodeList = []
        self.nodeIdDict = dict()

    # get max number of nodes
    def getMaxNodes(self):
        return self.maxNodes

    # get len of nodeList
    def getLenNodeList(self):
        return len(self.nodeList)

    # check if given identifier matches an existing node
    def isEcmu(self, identifier):
        if identifier in self.nodeIdDict.keys():
            return True
        else:
            return False

    # add a node to the list of nodes/ID LUT
    def addEcmu(self, conn, addr, identifier):
        self.nodeIdDict[identifier] = len(self.nodeList)
        self.nodeList.append(ecmu(conn, addr, identifier))

    # collect the node output assignments from a csv and append to outputList
    def collectOutputs(self):
        pass

    # receive data from all nodes
    def receive(self):
        for node in self.nodeList:
            node.receive()

    # analyze data from all nodes
    def analyze(self):
        for node in self.nodeList:
            node.analyze()

    # transmit data from all nodes
    def transmit(self):
        for node in self.nodeList:
            node.receive()

    # prints data from all nodes
    def print(self):
        for node in self.nodeList:
            print("Node: %s" % (node.getIdentifier()))
            print("IP Address: %s" % (node.getAddr()[0]))
            print("O2 Sensor Reading: %d" % (node.getO2()))
            print("Temperature Sensor Reading: %f" % (node.getTemp()))
            print("Fire Detected: %s" % (node.getFlame()))

            if (node.getAlert() == 1):
                print("ALERT")

            print("\n")

    def disconnect(self):
        for node in self.nodeList:
            node.getConn().close()


